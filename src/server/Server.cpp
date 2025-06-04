#include "Server.hpp"
#include "AMessage.hpp"
#include "Application.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <csignal>
#include <cstddef>
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
#include <unistd.h>
#include <utility>
#include <vector>

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

Server::~Server(void){};

extern "C" void callServerShutdown(int signal) {
	(void)signal;
	sigint = 1;
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
	std::cout << "TEST" << std::endl;
	_serverLoop();
}

void Server::_shutdown(void) {
	std::cout << "\nShutting down server" << std::endl;
	for (std::map<int, Application *>::iterator it = _clientAppMap.begin();
	     it != _clientAppMap.end(); ++it) {
		_disconnectClient(it->first);
	}
	for (std::vector<Application>::iterator it = _applicationList.begin();
	     it != _applicationList.end(); ++it) {
		it->close();
	}
	close(_epollfd);
}

bool Server::_checkServerState() {
	if (!sigint)
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
	std::pair<std::string, bool> transferEncodingValue =
	    request.getHeaderValue("Transfer-Encoding");
	if (!transferEncodingValue.second || transferEncodingValue.first != "chunked")
		return;

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
		std::cout << "-- chunk --\n" << result << std::endl;
		request.appendChunk(result);
	}
	std::cout << "--- REQUEST ---" << request.str() << std::endl;
}

RequestMessage Server::_listenClientRequest(int clientfd, unsigned long clientMaxBodySize) {
	const int     bufSize = 8192;
	char          buffer[bufSize];
	unsigned long count = 0;
	std::string   result;

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
		count += bytesRead;
		// std::cout << "result --\n" << result << std::endl;
		if (clientMaxBodySize != 0 && count >= clientMaxBodySize) {
			throw AMessage::MessageError(413);
		}
		if (result.find("\r\n\r\n") != std::string::npos)
			break;
	}
	RequestMessage request(result);
	_listenChunkedRequest(clientfd, request, clientMaxBodySize);
	return request;
}

Application &Server::_getApplicationFromFD(int sockfd) const { return *_clientAppMap.at(sockfd); }

void Server::_disconnectClient(int clientfd) {
	_clientAppMap.erase(clientfd);
	epoll_ctl(_epollfd, EPOLL_CTL_DEL, clientfd, NULL);
	close(clientfd);
}

void Server::_evaluateClientConnection(int clientfd, const ResponseMessage &response) {
	std::pair<std::string, bool> connectionValue = response.getHeaderValue("Connection");

	if (!connectionValue.second || connectionValue.first != "close")
		return;
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
				Config actualAppConfig = _getApplicationFromFD(events[i].data.fd).getConfig();
				try {
					RequestMessage request = _listenClientRequest(
					    events[i].data.fd, actualAppConfig.getClientMaxBodySize());
					ResponseMessage response =
					    RequestHandler::generateResponse(actualAppConfig, request);
					_sendAnswer(response.str(), events[i].data.fd);
					_evaluateClientConnection(clientfd, response);
				} catch (AMessage::MessageError &e) {
					ResponseMessage answer =
					    RequestHandler::generateErrorResponse(actualAppConfig, e.getStatusCode());
					_sendAnswer(answer.str(), events[i].data.fd);
				} catch (std::exception &e) {
					std::cerr << "Error: " << e.what() << std::endl;
					continue;
				}
			}
		}
	}
}
