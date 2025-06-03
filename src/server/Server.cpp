#include "Server.hpp"
#include "AMessage.hpp"
#include "Application.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
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

void Server::_initServer(void) {
	_epollfd = epoll_create(MAX_EVENTS);
	for (std::vector<Application>::iterator itServer = _applicationList.begin();
	     itServer != _applicationList.end(); ++itServer) {
		itServer->initApplication(_epollfd);
	}
}

void Server::startServer(void) {
	_initServer();
	std::cout << "TEST" << std::endl;
	_serverLoop();
}

void Server::_sendAnswer(const std::string &answer, int clientfd) {
	if (send(clientfd, answer.c_str(), answer.length(), MSG_NOSIGNAL) < 0) {
		std::cerr << "Error on write." << std::endl;
		_clientAppMap.erase(clientfd);
		close(clientfd);
	}
}

RequestMessage Server::_listenClientRequest(int clientfd, unsigned long clientMaxBodySize) {
	const int     bufSize = 8192;
	char          buffer[bufSize];
	unsigned long count = 0;
	std::string   result;

	bzero(buffer, bufSize);
	ssize_t bytesRed = 1;
	while (bytesRed) {
		bzero(buffer, bufSize);
		bytesRed = read(clientfd, buffer, bufSize);
		if (bytesRed < 0) {
			close(clientfd);
			throw std::runtime_error("error on read");
		}
		result.append(buffer, bytesRed);
		count += bytesRed;
		std::cout << "result --\n" << result << std::endl;
		if (clientMaxBodySize != 0 && count >= clientMaxBodySize) {
			throw AMessage::MessageError(413);
		}
		if (result.find("\r\n\r\n") != std::string::npos)
			break;
	}
	return RequestMessage(result);
}

Application &Server::_getApplicationFromFD(int sockfd) const { return *_clientAppMap.at(sockfd); }

void Server::_serverLoop() {
	struct epoll_event ev;
	int                clientfd = -1;
	int                nfds;
	struct epoll_event events[MAX_EVENTS];
	bool               newClient;

	while (true) {
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
					std::string requestStr;

					RequestMessage request = _listenClientRequest(
					    events[i].data.fd, actualAppConfig.getClientMaxBodySize());
					ResponseMessage answer =
					    RequestHandler::generateResponse(actualAppConfig, request);
					_sendAnswer(answer.str(), events[i].data.fd);
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
