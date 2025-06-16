#include "Server.hpp"
#include "AMessage.hpp"
#include "Application.hpp"
#include "CgiHandler.hpp"
#include "Client.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

int g_sigint = 0;

Server::Server(const std::string &filename) {
	std::fstream file;
	file.open(filename.c_str(), std::fstream::in);
	if (!file.is_open()) {
		throw Config::Exception("Problem opening file");
	}
	try {
		while (true) {
			_applicationList.push_back(Application(file));
		}
	} catch (Config::Finished &e) {
		if (_applicationList.empty())
			throw Config::Exception("Empty config file"); // close file ?
	}
	file.close();
}

Server::~Server(void) {};

extern "C" void callServerShutdown(int signal) {
	(void)signal;
	g_sigint = 1;
}

bool Server::_initServer(void) {
	// Check si servers ont des ports disctincts
	for (std::vector<Application>::iterator it = _applicationList.begin();
	     it != _applicationList.end(); ++it) {
		std::vector<Application>::iterator it2 = it;
		it2++;
		while (it2 != _applicationList.end()) {
			if (it->getConfig().getPort() == it2->getConfig().getPort()) {
				std::cout << "Error : Multiple servers with the same port\n";
				return false;
			}
			it2++;
		}
	}

	_epollfd = epoll_create(MAX_EVENTS);
	for (std::vector<Application>::iterator itServer = _applicationList.begin();
	     itServer != _applicationList.end(); ++itServer) {
		itServer->initApplication(_epollfd);
	}
	signal(SIGINT, callServerShutdown);
	return true;
}

void Server::startServer(void) {
	if (_initServer())
		_serverLoop();
}

void Server::_shutdown(void) {
	std::cout << "\nShutting down server" << std::endl;
	for (std::map<int, Client>::iterator it = _clientMap.begin(); it != _clientMap.end(); ++it) {
		_disconnectClient(it->first);
	}
	_clientMap.clear();
	for (std::vector<Application>::iterator it = _applicationList.begin();
	     it != _applicationList.end(); ++it) {
		it->close();
	}
	close(_epollfd);
}

bool Server::_checkServerState() {
	if (!g_sigint)
		return false;
	_shutdown();
	return true;
}

bool Server::_sendAnswer(s_connection &con) {
	if (con.bufferWrite.empty()) {
		return true;
	}

	size_t      ttSize = con.bufferWrite.length();
	size_t      sizeLeft = ttSize - con.bytesWritten;
	const char *bufferPos = con.bufferWrite.c_str() + con.bytesWritten;

	ssize_t bytesSent = send(con.clientFd, bufferPos, sizeLeft, MSG_NOSIGNAL);
	if (bytesSent > 0) {
		con.bytesWritten += bytesSent;
	} else if (bytesSent == -1) {
		std::cerr << "Error on write\n";
	}
	if (con.bytesWritten >= ttSize) {
		return true;
	}
	return false;
}

void Server::_listenClientRequest(int clientfd, unsigned long clientMaxBodySize) {
	const int     bufSize = 8192;
	char          buffer[bufSize];
	s_connection *con = &connections[clientfd];
	Client        client(clientfd);
	_clientMap.insert(std::pair<int, Client &>(clientfd, client));

	ssize_t bytesRead = 1;

	bzero(buffer, bufSize);
	bytesRead = read(clientfd, buffer, bufSize);
	if (bytesRead < 0) {
		close(clientfd);
		throw std::runtime_error("error on read");
	}
	if (bytesRead == 0) {
		std::cout << "[LIFECYCLE] FD " << clientfd << ": DISCONNECTED BY CLIENT (read=0)"
		          << std::endl;
		_cleanupConnection(clientfd);
		return;
	}
	if (con->bytesToRead != -1) {
		if (con->bytesToRead < bytesRead)
			throw AMessage::MessageError(413);
		con->bytesToRead -= bytesRead;
	}
	if (clientMaxBodySize != 0 && con->bufferRead.length() > clientMaxBodySize)
		throw AMessage::MessageError(413);
	con->bufferRead.append(buffer, bytesRead);
	if (!con->bytesToRead) {
		requestMap[clientfd] = RequestMessage(connections[clientfd].bufferRead);
		con->status = PROCESSING;
	}
	if (con->chunk) {
		if (con->bufferRead.find("0\r\n\r\n") != std::string::npos) {
			// Recree pour omettre ce qui peut etre apres 0\r\n\r\n
			requestMap[clientfd] =
			    RequestMessage(con->bufferRead.substr(0, con->bufferRead.find("0\r\n\r\n") + 5));
			con->status = PROCESSING;
		}
	}

	if (con->bytesToRead == -1 && !con->chunk) {
		if (con->bufferRead.find("\r\n\r\n") != std::string::npos) {
			RequestMessage request(connections[clientfd].bufferRead);
			if (request.getHeaderValue("Content-Length").second) {
				// verifier que first de content length est bon
				std::string value = request.getHeaderValue("Content-Length").first;
				if (value.find_first_not_of("0123456789") != std::string::npos ||
				    value.length() >= 10)
					throw AMessage::MessageError(400);
				con->bytesToRead = atoi(request.getHeaderValue("Content-Length").first.c_str()) -
				                   request.getBody().size();
				if (con->bytesToRead < 0)
					throw AMessage::MessageError(413);
				if (con->bytesToRead == 0) {
					requestMap[clientfd] = request;
					con->status = PROCESSING;
				}
			} else if (request.getHeaderValue("Transfer-Encoding").second &&
			           request.getHeaderValue("Transfer-Encoding").first == "chunked") {
				if (con->bufferRead.find("0\r\n\r\n") != std::string::npos) {
					// Recree pour omettre ce qui peut etre apres 0\r\n\r\n
					requestMap[clientfd] = RequestMessage(
					    con->bufferRead.substr(0, con->bufferRead.find("0\r\n\r\n") + 5));
					con->status = PROCESSING;
				}
				con->chunk = true;
			} else {
				requestMap[clientfd] = RequestMessage(connections[clientfd].bufferRead);
				if (!requestMap[clientfd].getBody().empty())
					throw AMessage::MessageError(400);
				con->status = PROCESSING;
			}
		}
	}
}

Application &Server::_getApplicationFromFD(int sockfd) const {
	return _clientMap.at(sockfd).getApplication();
}

void Server::_disconnectClient(int clientfd) const {
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, clientfd, NULL);
	close(clientfd);
}

bool Server::_evaluateClientConnection(int clientfd, const ResponseMessage &response) {
	std::pair<std::string, bool> connectionValue = response.getHeaderValue("Connection");

	if (!connectionValue.second || connectionValue.first != "close")
		return false;
	_clientMap.erase(clientfd);
	_disconnectClient(clientfd);
	return true;
}

void Server::_modifySocketEpoll(int epollfd, int clientfd, int flags) {
	epoll_event ev;
	ev.events = flags;
	ev.data.fd = clientfd;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, clientfd, &ev);
}

bool Server::_acceptClientConnection(int currentFd, int &clientfd) {
	for (std::vector<Application>::iterator it = _applicationList.begin();
	     it != _applicationList.end(); ++it) {
		if (currentFd != it->getLSockFd())
			continue;
		while (true) {
			clientfd = accept(currentFd, NULL, NULL);
			if (clientfd < 0) {
				if (errno != EAGAIN)
					std::cerr << "Error on accept clients." << std::endl;
				break;
			}
			std::cout << "[LIFECYCLE] FD " << clientfd << ": CREATED" << std::endl;
			_clientMap[clientfd] = Client(clientfd);
			_clientMap[clientfd].setApplication(&(*it));

			connections[clientfd] = s_connection(clientfd);
			struct epoll_event ev;
			ev.events = REQUEST_FLAGS;
			ev.data.fd = clientfd;
			epoll_ctl(_epollfd, EPOLL_CTL_ADD, clientfd, &ev);
		}
		return true;
	}
	return false;
}

void Server::_serverLoop() {
	int                clientfd = -1;
	int                nfds;
	struct epoll_event events[MAX_EVENTS];

	while (true) {
		if (_checkServerState())
			break;
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, TIME_OUT);

		for (int i = 0; i < nfds; ++i) {
			int  currentFd = events[i].data.fd;
			bool isNewClient = _acceptClientConnection(events[i].data.fd, clientfd);
			if (isNewClient)
				continue;
			// si ne fonctionne pas, remplacer clientfd par events[i].data.fd
			try {
				if (cgiSessions.count(currentFd)) {
					_handleActiveCgi(events[i]);
					continue;
				}
				if (events[i].events & EPOLLIN) {
					Config actualAppConfig = _getApplicationFromFD(currentFd).getConfig();
					_listenClientRequest(currentFd, actualAppConfig.getClientMaxBodySize());
					s_connection *con = &connections[currentFd];
					if (con->status == PROCESSING) {
						responseMap[currentFd] = RequestHandler::generateResponse(
						    actualAppConfig, requestMap[currentFd], currentFd);
						con->bufferWrite = responseMap[currentFd].str();
						con->status = WRITING_OUTPUT;
						_modifySocketEpoll(_epollfd, currentFd, RESPONSE_FLAGS);
					}
				} else if (events[i].events & EPOLLOUT) {
					s_connection *con = &connections[currentFd];
					if (con->status == WRITING_OUTPUT) {
						bool doneSending = _sendAnswer(*con);
						if (!doneSending)
							continue;
						if (!_evaluateClientConnection(currentFd, responseMap[currentFd])) {
							_clearForNewRequest(currentFd);
							_modifySocketEpoll(_epollfd, currentFd, REQUEST_FLAGS);
						} else
							_cleanupConnection(currentFd);
					}
				}
			} catch (RequestHandler::CgiRequestException &e) {
				cgiSessions[currentFd] = cgiSession(events[i].data.fd, e.request, events[i]);
				CgiHandler::executeCgi(e.uri, e.config, cgiSessions[currentFd], *this,
				                       events[i].data.fd);
			} catch (AMessage::MessageError &e) {
				responseMap[currentFd] = RequestHandler::generateErrorResponse(
				    _getApplicationFromFD(currentFd).getConfig(), e.getStatusCode());
				connections[currentFd] = s_connection(currentFd);
				connections[currentFd].bufferWrite = responseMap[currentFd].str();
				connections[currentFd].status = WRITING_OUTPUT;
				_modifySocketEpoll(_epollfd, currentFd, RESPONSE_FLAGS);
			} catch (std::exception &e) {
				std::cerr << "Error in handling request: " << e.what() << std::endl;
				_cleanupConnection(currentFd);
			}
		}
	}
}

int Server::getEpollFd() const { return this->_epollfd; }

void Server::_handleActiveCgi(struct epoll_event &event) {
	int activeFd = event.data.fd;

	// 1. Récupérer la session CGI associée à ce fd
	if (cgiSessions.find(activeFd) == cgiSessions.end()) {
		// Ne devrait pas arriver, mais par sécurité on nettoie
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, activeFd, NULL);
		close(activeFd);
		return;
	}
	int clientFd = cgiSessions[activeFd].getClientFd();

	// Gestion erreurs
	if (event.events & (EPOLLERR | EPOLLHUP)) {
		_cleanupCgiSession(cgiSessions[clientFd]);
		return;
	}

	// Ecriture pipeFdIn
	if (activeFd == cgiSessions[clientFd].getPipeToCgi() && (event.events & EPOLLOUT)) {
		size_t bytesToWrite = cgiSessions[clientFd].request.getBody().length() -
		                      cgiSessions[clientFd].bytesWrittenToCgi;
		if (bytesToWrite == 0) {
			_stopWritingToCgi(cgiSessions[clientFd]);
			return;
		}
		std::string requestBody = cgiSessions[clientFd].request.getBody();
		const char *buffer = requestBody.c_str() + cgiSessions[clientFd].bytesWrittenToCgi;
		ssize_t     bytesWritten = write(activeFd, buffer, bytesToWrite);

		if (bytesWritten > 0) {
			cgiSessions[clientFd].bytesWrittenToCgi += bytesWritten;
		} else if (bytesWritten == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			return;
		} else {
			std::cerr << "Erreur d'écriture sur le pipe du CGI" << std::endl;
			_cleanupCgiSession(cgiSessions[clientFd]);
			return;
		}

		if (cgiSessions[clientFd].bytesWrittenToCgi >=
		    cgiSessions[clientFd].request.getBody().length()) {
			_stopWritingToCgi(cgiSessions[clientFd]);
		}
	}

	// Lecture PipeFdOut
	else if (activeFd == cgiSessions[clientFd].getPipeFromCgi() && (event.events & EPOLLIN)) {
		char    buffer[4096];
		ssize_t bytesRead = 0;

		while (true) {
			bytesRead = read(activeFd, buffer, sizeof(buffer));

			if (bytesRead > 0) {
				cgiSessions[clientFd].cgiResponse.append(buffer, bytesRead);
			} else
				break;
		}
		if (bytesRead == 0) {
			_finalizeCgiRead(cgiSessions[cgiSessions[activeFd].getClientFd()]);
		} else {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				std::cerr << "Erreur de lecture depuis le pipe du CGI" << std::endl;
				_cleanupCgiSession(cgiSessions[clientFd]);
			}
		}
	}
}

void Server::_stopWritingToCgi(cgiSession &session) {
	if (session.getPipeToCgi() != -1) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, session.getPipeToCgi(), NULL);
		close(session.getPipeToCgi());
		cgiSessions.erase(session.getPipeToCgi());
	}
}

void Server::_stopReadingFromCgi(cgiSession &session) {
	int clientFd = session.getClientFd();
	if (session.getPipeFromCgi() != -1) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, session.getPipeFromCgi(), NULL);
		close(session.getPipeFromCgi());
		cgiSessions.erase(session.getPipeFromCgi());
	}

	// Maintenant que la réponse est prête, on change la surveillance sur le client
	// pour ÉCRIRE au lieu de LIRE.
	struct epoll_event ev;
	ev.events = EPOLLOUT | EPOLLET; // On veut être notifié quand on peut écrire au client
	ev.data.fd = clientFd;
	epoll_ctl(_epollfd, EPOLL_CTL_MOD, clientFd, &ev);
}

void Server::_cleanupCgiSession(cgiSession &session) {
	int clientFd = session.getClientFd();

	if (session.getPipeToCgi() != -1)
		_stopWritingToCgi(session);
	if (session.getPipeFromCgi() != -1)
		_stopReadingFromCgi(session);

	// Retirer le client de epoll et le fermer
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, clientFd, NULL);
	close(clientFd);

	if (session.getCgiPid() > 0) {
		int status;
		waitpid(session.getCgiPid(), &status, WNOHANG); // WNOHANG pour ne pas bloquer
	}

	// Retirer les FDs de la map de suivi (ceux de to et from cgi sont supp dans fonction stop..)
	cgiSessions.erase(clientFd);
	_clientMap.erase(clientFd);

	if (connections.count(clientFd)) {
		connections.erase(clientFd);
	}
}

void Server::_finalizeCgiRead(cgiSession &session) {
	if (session.getCgiPid() > 0) {
		int status;
		waitpid(session.getCgiPid(), &status, WNOHANG);
		session.setCgiPid(-1);
	}
	if (session.getPipeFromCgi() != -1) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, session.getPipeFromCgi(), NULL);
		close(session.getPipeFromCgi());

		cgiSessions.erase(session.getPipeFromCgi());
	}
	if (session.getPipeToCgi() != -1) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, session.getPipeToCgi(), NULL);
		close(session.getPipeToCgi());
		cgiSessions.erase(session.getPipeToCgi());
	}

	s_connection *con = &connections[session.getClientFd()];
	if (con) {
		StatusLine      statusLine = RequestHandler::_generateStatusLine(200);
		ResponseMessage response(statusLine, session.cgiResponse);
		RequestHandler::_generateHeaders(response, session.request, 200);
		con->bufferWrite = response.str();
		con->status = WRITING_OUTPUT;
		_modifySocketEpoll(_epollfd, session.getClientFd(), RESPONSE_FLAGS);
		cgiSessions.erase(session.getClientFd());
	}
}

void Server::_cleanupConnection(int fd) {
	if (cgiSessions.count(fd)) {
		_cleanupCgiSession(cgiSessions[fd]);
		return;
	}
	std::cout << "[LIFECYCLE] FD " << fd << ": DESTROYED" << std::endl;
	connections.erase(fd);
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	_clientMap.erase(fd);
}

void Server::_clearForNewRequest(int clientFd) {
	std::cout << "[LIFECYCLE] FD " << clientFd << ": RESET (Keep-Alive)" << std::endl;
	requestMap[clientFd] = RequestMessage();
	responseMap[clientFd] = ResponseMessage();
	connections[clientFd].bufferRead.clear();
	connections[clientFd].bufferWrite.clear();
	connections[clientFd].bytesWritten = 0;
	connections[clientFd].bytesToRead = -1;
	connections[clientFd].status = FINISHED;
	connections[clientFd].chunk = false;
}
