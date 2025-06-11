#include "Server.hpp"
#include "AMessage.hpp"
#include "Application.hpp"
#include "CgiHandler.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <sys/epoll.h>
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

void Server::_initServer(void) {
	_epollfd = epoll_create(MAX_EVENTS);
	for (std::vector<Application>::iterator itServer = _applicationList.begin();
	     itServer != _applicationList.end(); ++itServer) {
		itServer->initApplication(_epollfd);
	}
	signal(SIGINT, callServerShutdown);
}

void Server::startServer(void) {
	_initServer();
	_serverLoop();
}

void Server::_shutdown(void) {
	std::cout << "\nShutting down server" << std::endl;
	for (std::map<int, Application *>::iterator it = _clientAppMap.begin();
	     it != _clientAppMap.end(); ++it) {
		_disconnectClient(it->first);
	}
	_clientAppMap.clear();
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

void Server::_sendAnswer(const std::string &answer, int clientfd) {
	if (send(clientfd, answer.c_str(), answer.length(), MSG_NOSIGNAL) < 0) {
		std::cerr << "Error on write." << std::endl;
		_clientAppMap.erase(clientfd);
		close(clientfd);
	}
}

void Server::_listenChunkedRequest(int clientfd, RequestMessage &request,
                                   unsigned long clientMaxBodySize) {
	const int bufSize = 8192;
	char      buffer[bufSize];

	while (true) {
		std::string result;
		bzero(buffer, bufSize);
		ssize_t bytesRead = 1;

		while (bytesRead) {
			bzero(buffer, bufSize);
			bytesRead = read(clientfd, buffer, bufSize);
			if (bytesRead < 0) {
				close(clientfd);
				throw std::runtime_error("error on read");
			}
			result.append(buffer, bytesRead);
			if (result.find("\r\n") != result.rfind("\r\n") || result.find("\r\n") == 0)
				break;
		}
		if (result == "0\r\n\r\n")
			break;
		if (clientMaxBodySize != 0 && result.length() >= clientMaxBodySize)
			throw AMessage::MessageError(413);
		request.appendChunk(result);
	}
}

RequestMessage Server::_listenClientRequest(int clientfd, unsigned long clientMaxBodySize) {
	char          c;
	unsigned long count = 0;
	std::string   result;

	ssize_t bytesRead = 1;
	while (bytesRead) {
		c = 0;
		bytesRead = read(clientfd, &c, 1);
		if (bytesRead < 0) {
			close(clientfd);
			throw std::runtime_error("error on read");
		}
		result += c;
		count += bytesRead;
		if (clientMaxBodySize != 0 && count >= clientMaxBodySize) {
			throw AMessage::MessageError(413);
		}
		if (result.find("\r\n") == 0) {
			result = result.substr(2);
			count -= 2;
		}
		if (result.find("\r\n\r\n") != std::string::npos)
			break;
	}
	RequestMessage request(result);

	if (request.getHeaderValue("Transfer-Encoding").second &&
	    request.getHeaderValue("Transfer-Encoding").first == "chunked")
		_listenChunkedRequest(clientfd, request, clientMaxBodySize);
	else if (request.getHeaderValue("Content-Length").second)
		_listenBody(clientfd, request, clientMaxBodySize ? clientMaxBodySize - count : 0);
	return request;
}

void Server::_listenBody(int clientfd, RequestMessage &request, unsigned long sizeLeft) {
	std::string body;
	ssize_t     bytesRead = 1;
	char        c;
	ssize_t     totalBody = 0;
	int         contentLength = atoi(request.getHeaderValue("Content-Length").first.c_str());

	while (bytesRead && totalBody < contentLength) {
		c = 0;
		bytesRead = read(clientfd, &c, 1);
		if (bytesRead < 0) {
			close(clientfd);
			throw std::runtime_error("error on read");
		}
		body += c;
		totalBody += bytesRead;
		if (sizeLeft) {
			sizeLeft -= bytesRead;
			if (!sizeLeft)
				throw AMessage::MessageError(413);
		}
	}
	request.setBody(body);
}

Application &Server::_getApplicationFromFD(int sockfd) const { return *_clientAppMap.at(sockfd); }

void Server::_disconnectClient(int clientfd) const {
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, clientfd, NULL);
	close(clientfd);
}

void Server::_evaluateClientConnection(int clientfd, const ResponseMessage &response) {
	std::pair<std::string, bool> connectionValue = response.getHeaderValue("Connection");

	if (!connectionValue.second || connectionValue.first != "close")
		return;
	_clientAppMap.erase(clientfd);
	_disconnectClient(clientfd);
}

void Server::_serverLoop() {
	struct epoll_event ev;
	int                clientfd = -1;
	int                nfds;
	struct epoll_event events[MAX_EVENTS];
	bool               newClient;

	while (true) {
		if (_checkServerState())
			break;
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, TIME_OUT);

		for (int i = 0; i < nfds; ++i) {
			newClient = false;
			for (std::vector<Application>::iterator itServer = _applicationList.begin();
			     itServer != _applicationList.end(); ++itServer) {
				if (events[i].data.fd == itServer->getLSockFd()) {
					clientfd = accept(itServer->getLSockFd(), NULL, NULL);
					if (clientfd < 0) {
						std::cerr << "Error on accept clients." << std::endl;
						continue;
					}

					_clientAppMap[clientfd] = &(*itServer);

					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = clientfd;
					epoll_ctl(_epollfd, EPOLL_CTL_ADD, clientfd, &ev);
					newClient = true;
				}
			}
			if (!newClient) {
				if (cgiSessions.count(events[i].data.fd)) {
					_handleActiveCgi(events[i]); /// a coder
				} else {
					try {
						Config actualAppConfig =
						    _getApplicationFromFD(events[i].data.fd).getConfig();
						RequestMessage request = _listenClientRequest(
						    events[i].data.fd, actualAppConfig.getClientMaxBodySize());
						ResponseMessage response =
						    RequestHandler::generateResponse(actualAppConfig, request, clientfd);
						_sendAnswer(response.str(), events[i].data.fd);
						_evaluateClientConnection(clientfd, response);
					} catch (RequestHandler::CgiRequestException &e) {
						CgiHandler::executeCgi(e.request, e.uri, e.config, *this, e.clientFd);
					} catch (AMessage::MessageError &e) {
						ResponseMessage response = RequestHandler::generateErrorResponse(
						    _getApplicationFromFD(events[i].data.fd).getConfig(),
						    e.getStatusCode());
						_sendAnswer(response.str(), events[i].data.fd);
						_cleanupConnection(events[i].data.fd);
					} catch (std::exception &e) {
						std::cerr << "Error in handling request: " << e.what() << std::endl;
						_cleanupConnection(events[i].data.fd);
						continue;
					}
				}
			}
		}
	}
}

int Server::getEpollFd() const { return this->_epollfd; }

void Server::_handleActiveCgi(const struct epoll_event &event) {
	int activeFd = event.data.fd;

	// 1. Récupérer la session CGI associée à ce fd
	if (cgiSessions.find(activeFd) == cgiSessions.end()) {
		// Ne devrait pas arriver, mais par sécurité on nettoie
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, activeFd, NULL);
		close(activeFd);
		return;
	}
	s_cgiSession *session = cgiSessions[activeFd];

	// Gestion erreurs
	if (event.events & (EPOLLERR | EPOLLHUP)) {
		_cleanupCgiSession(session);
		return;
	}

	// Ecriture pipeFdIn
	if (activeFd == session->pipeToCgi && (event.events & EPOLLOUT)) {
		size_t bytesToWrite = session->requestBody.length() - session->bytesWrittenToCgi;
		if (bytesToWrite == 0) {
			_stopWritingToCgi(session);
			return;
		}

		const char *buffer = session->requestBody.c_str() + session->bytesWrittenToCgi;
		ssize_t     bytesWritten = write(activeFd, buffer, bytesToWrite);

		if (bytesWritten > 0) {
			session->bytesWrittenToCgi += bytesWritten;
		} else if (bytesWritten == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			return;
		} else {
			std::cerr << "Erreur d'écriture sur le pipe du CGI" << std::endl;
			_cleanupCgiSession(session);
			return;
		}

		if (session->bytesWrittenToCgi >= session->requestBody.length()) {
			_stopWritingToCgi(session);
		}
	}

	// Lecture PipeFdOut
	else if (activeFd == session->pipeFromCgi && (event.events & EPOLLIN)) {
		while (true) {
			char    buffer[4096];
			ssize_t bytesRead = read(activeFd, buffer, sizeof(buffer));

			if (bytesRead > 0) {
				session->cgiResponse.append(buffer, bytesRead);
			} else if (bytesRead == 0) {
				_stopReadingFromCgi(session);
				break;
			} else {
				if (errno != EAGAIN && errno != EWOULDBLOCK) {
					std::cerr << "Erreur de lecture depuis le pipe du CGI" << std::endl;
					_cleanupCgiSession(session);
				}
			}
		}
	}

	// ÉCRIRE la réponse finale au CLIENT
	else if (activeFd == session->clientFd && (event.events & EPOLLOUT)) {
		StatusLine      statusLine = RequestHandler::_generateStatusLine(200);
		ResponseMessage response(statusLine, session->cgiResponse);
		RequestHandler::_generateHeaders(response, session->request, 200);
		_sendAnswer(response.str(), session->clientFd);
		_evaluateClientConnection(session->clientFd, response);
		_cleanupCgiSession(session);
	}
}

void Server::_stopWritingToCgi(s_cgiSession *session) {
	if (session->pipeToCgi != -1) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, session->pipeToCgi, NULL);
		close(session->pipeToCgi);
		cgiSessions.erase(session->pipeToCgi);
		session->pipeToCgi = -1;
	}
}

void Server::_stopReadingFromCgi(s_cgiSession *session) {
	if (session->pipeFromCgi != -1) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, session->pipeFromCgi, NULL);
		close(session->pipeFromCgi);
		cgiSessions.erase(session->pipeFromCgi);
		session->pipeFromCgi = -1;
	}

	// Maintenant que la réponse est prête, on change la surveillance sur le client
	// pour ÉCRIRE au lieu de LIRE.
	struct epoll_event ev;
	ev.events = EPOLLOUT | EPOLLET; // On veut être notifié quand on peut écrire au client
	ev.data.fd = session->clientFd;
	epoll_ctl(_epollfd, EPOLL_CTL_MOD, session->clientFd, &ev);
}

void Server::_cleanupCgiSession(s_cgiSession *session) {
	if (!session)
		return;

	if (session->pipeToCgi != -1)
		_stopWritingToCgi(session);
	if (session->pipeFromCgi != -1)
		_stopReadingFromCgi(session);

	// Retirer le client de epoll et le fermer
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, session->clientFd, NULL);
	close(session->clientFd);

	// Retirer les FDs de la map de suivi
	cgiSessions.erase(session->pipeToCgi);
	cgiSessions.erase(session->pipeFromCgi);
	cgiSessions.erase(session->clientFd);
	_clientAppMap.erase(session->clientFd);

	if (session->cgiPid > 0) {
		int status;
		waitpid(session->cgiPid, &status, WNOHANG); // WNOHANG pour ne pas bloquer
	}
	delete session;
}

void Server::_cleanupConnection(int fd) {
	if (cgiSessions.count(fd)) {
		s_cgiSession *session = cgiSessions[fd];
		_cleanupCgiSession(session);
		return;
	}
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	_clientAppMap.erase(fd);
}
