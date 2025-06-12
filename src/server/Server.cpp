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
#include <fcntl.h>
#include <iostream>
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
		delete connections[it->first];
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

bool Server::_sendAnswer(s_connection &con, struct epoll_event &ev,
                         const ResponseMessage &response) {
	(void)response;
	(void)ev;
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
	s_connection *con = connections[clientfd];

	ssize_t bytesRead = 1;

	bzero(buffer, bufSize);
	bytesRead = read(clientfd, buffer, bufSize);
	if (bytesRead < 0) {
		close(clientfd);
		throw std::runtime_error("error on read");
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
		requestMap[clientfd] = RequestMessage(connections[clientfd]->bufferRead);
		con->status = PROCESSING;
	}
	if (con->chunk) {
		if (con->bufferRead.find("0\r\n\r\n") != std::string::npos) {
			// ajouter check si c'est bien la fin ?
			requestMap[clientfd] = RequestMessage(connections[clientfd]->bufferRead);
			con->status = PROCESSING;
		}
	}

	if (con->bytesToRead == -1) {
		if (con->bufferRead.find("\r\n\r\n") != std::string::npos) {
			if (con->bufferRead.find("Content-Length") != std::string::npos) {
				// recuperer la valeur puis changer bytesToRead
			} else if (con->bufferRead.find("chunk jspu") != std::string::npos) {
				// recuperer la valeur puis changer chunk si bonne valeur
			} else {
				// si aucun des 2, verifier que \r\n\r\n est a la fin (pas de body)
				requestMap[clientfd] = RequestMessage(connections[clientfd]->bufferRead);
				con->status = PROCESSING;
			}
		}
	}

	/*
	    if (result.find("\r\n") != result.rfind("\r\n") || result.find("\r\n") == 0)
	        break;

	// POur eviter les \r\n au tout debut
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
	*/
}

Application &Server::_getApplicationFromFD(int sockfd) const { return *_clientAppMap.at(sockfd); }

void Server::_disconnectClient(int clientfd) const {
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, clientfd, NULL);
	close(clientfd);
}

bool Server::_evaluateClientConnection(int clientfd, const ResponseMessage &response) {
	std::pair<std::string, bool> connectionValue = response.getHeaderValue("Connection");

	if (!connectionValue.second || connectionValue.first != "close")
		return 0;
	_clientAppMap.erase(clientfd);
	_disconnectClient(clientfd);
	return 1;
}

void Server::_modifySocketEpoll(int epollfd, int clientfd, int flags) {
	epoll_event ev;
	ev.events = flags;
	ev.data.fd = clientfd;
	epoll_ctl(epollfd, EPOLL_CTL_MOD, clientfd, &ev);
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

					connections[clientfd] = new s_connection(clientfd);
					ev.events = REQUEST_FLAGS;
					ev.data.fd = clientfd;
					epoll_ctl(_epollfd, EPOLL_CTL_ADD, clientfd, &ev);
					newClient = true;
					break;
				}
			}
			if (newClient)
				continue;

			s_connection *con = connections[events[i].data.fd];
			try {
				if (cgiSessions.count(events[i].data.fd)) {
					std::cout << "Rentre dans _handleActiveCgi\n";
					_handleActiveCgi(events[i],
					                 connections[cgiSessions[events[i].data.fd]->clientFd]);
				} else {
					if (events[i].events & EPOLLIN) {
						Config actualAppConfig =
						    _getApplicationFromFD(events[i].data.fd).getConfig();
						_listenClientRequest(events[i].data.fd,
						                     actualAppConfig.getClientMaxBodySize());
						if (con->status == PROCESSING) {
							responseMap[clientfd] = RequestHandler::generateResponse(
							    actualAppConfig, requestMap[clientfd], clientfd);
							con->bufferWrite = responseMap[clientfd].str();
							con->status = WRITING_OUTPUT;
							_modifySocketEpoll(_epollfd, events[i].data.fd, RESPONSE_FLAGS);
						}
					} else if (events[i].events & EPOLLOUT) {
						std::cout << "Rentre dans event pret A recevoir reponse\n";
						if (con->status == WRITING_OUTPUT) {
							std::cout << "Rentre dans WRITING_OUTPUT ok\n";
							bool doneSending = _sendAnswer(*con, ev, responseMap[clientfd]);
							if (doneSending) {
								if (!_evaluateClientConnection(clientfd, responseMap[clientfd])) {
									_clearForNewRequest(clientfd);
									_modifySocketEpoll(_epollfd, events[i].data.fd, REQUEST_FLAGS);
								} else {
									_cleanupConnection(clientfd);
								}
							}
						}
					}
				}
			} catch (RequestHandler::CgiRequestException &e) {
				CgiHandler::executeCgi(e.request, e.uri, e.config, *this, events[i]);
			} catch (AMessage::MessageError &e) {
				responseMap[events[i].data.fd] = RequestHandler::generateErrorResponse(
				    _getApplicationFromFD(events[i].data.fd).getConfig(), e.getStatusCode());
				con->bufferWrite = responseMap[events[i].data.fd].str();
				con->status = WRITING_OUTPUT;
				_modifySocketEpoll(_epollfd, clientfd, RESPONSE_FLAGS);
			} catch (std::exception &e) {
				std::cerr << "Error in handling request: " << e.what() << std::endl;
				_cleanupConnection(events[i].data.fd);
				continue;
			}
		}
	}
}

int Server::getEpollFd() const { return this->_epollfd; }

void Server::_handleActiveCgi(struct epoll_event &event, s_connection *con) {
	int activeFd = event.data.fd;

	std::cout << "Dans handle active cgi\n";
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
		std::cout << "Dans ecriture pipefdin\n";
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
		std::cout << "\n[EPOLLIN sur pipe " << activeFd << "] Début de la lecture..." << std::endl;
		char    buffer[4096];
		ssize_t bytesRead = 0;

		while (true) {
			bytesRead = read(activeFd, buffer, sizeof(buffer));

			if (bytesRead > 0) {
				session->cgiResponse.append(buffer, bytesRead);
				std::cout << "--- BUUFFER LU  ---- \n" << buffer << "\n\n---FIN BUFFER---";
				std::cout << "--- CGI RESPONSE ---- \n"
				          << session->cgiResponse << "\n\n---FIN CGIRESPONSE---";
			} else
				break;
		}
		std::cout << "Sorti de la boucle, bytesRead =" << bytesRead << std::endl;
		(void)con;
		if (bytesRead == 0) {
			_finalizeCgiRead(session);
		} else {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				std::cerr << "Erreur de lecture depuis le pipe du CGI" << std::endl;
				_cleanupCgiSession(session);
			}
		}
	}
	/*
	    // ÉCRIRE la réponse finale au CLIENT
	    else if (activeFd == session->clientFd && (event.events & EPOLLOUT)) {
	        StatusLine      statusLine = RequestHandler::_generateStatusLine(200);
	        ResponseMessage response(statusLine, session->cgiResponse);
	        RequestHandler::_generateHeaders(response, session->request, 200);
	        _modifySocketEpoll(_epollfd, session->clientFd, RESPONSE_FLAGS);
	        _sendAnswer(*con, session->event, responseMap[session->clientFd]);
	        _evaluateClientConnection(session->clientFd, response);
	        _cleanupCgiSession(session);
	    }
	*/
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

	int clientFd = session->clientFd;

	if (session->pipeToCgi != -1)
		_stopWritingToCgi(session);
	if (session->pipeFromCgi != -1)
		_stopReadingFromCgi(session);

	// Retirer le client de epoll et le fermer
	//	delete connections[session->clientFd];
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

	if (connections.count(clientFd)) {
		delete connections[clientFd];
		connections.erase(clientFd);
	}

	delete session;
}

void Server::_finalizeCgiRead(s_cgiSession *session) {
	if (!session)
		return;
	if (session->cgiPid > 0) {
		int status;
		waitpid(session->cgiPid, &status, WNOHANG);
		session->cgiPid = -1;
	}
	if (session->pipeFromCgi != -1) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, session->pipeFromCgi, NULL);
		close(session->pipeFromCgi);

		cgiSessions.erase(session->pipeFromCgi);
		session->pipeFromCgi = -1;
	}
	if (session->pipeToCgi != -1) {
		cgiSessions.erase(session->pipeToCgi);
		close(session->pipeToCgi);
		session->pipeToCgi = -1;
	}

	s_connection *con = connections[session->clientFd];
	if (con) {
		StatusLine      statusLine = RequestHandler::_generateStatusLine(200);
		ResponseMessage response(statusLine, session->cgiResponse);
		RequestHandler::_generateHeaders(response, session->request, 200);
		con->bufferWrite = response.str();
		con->status = WRITING_OUTPUT;
		_modifySocketEpoll(_epollfd, session->clientFd, RESPONSE_FLAGS);
		cgiSessions.erase(session->clientFd);
		delete session;
	}
}

void Server::_cleanupConnection(int fd) {
	if (cgiSessions.count(fd)) {
		s_cgiSession *session = cgiSessions[fd];
		_cleanupCgiSession(session);
		return;
	}
	delete connections[fd];
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	_clientAppMap.erase(fd);
}

void Server::_clearForNewRequest(int clientFd) {
	requestMap[clientFd] = RequestMessage();
	responseMap[clientFd] = ResponseMessage();
	connections[clientFd]->bufferRead.clear();
	connections[clientFd]->bufferWrite.clear();
	connections[clientFd]->bytesWritten = 0;
	connections[clientFd]->bytesToRead = -1;
	connections[clientFd]->status = FINISHED;
	connections[clientFd]->chunk = false;
}
