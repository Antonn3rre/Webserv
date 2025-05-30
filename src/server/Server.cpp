#include "Server.hpp"
#include "Application.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <exception>
#include <iostream>
#include <stdlib.h>
#include <string.h>
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

Server::~Server(void) {};

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

bool Server::_listenClientResponse(char *buffer, int clientfd) {
	bzero(buffer, 8192);
	if (read(clientfd, buffer, 8192) < 0) {
		std::cerr << "Error on read." << std::endl;
		close(clientfd);
		return true;
	}
	return false;
}

Application &Server::_getApplicationFromFD(int sockfd) const { return *_clientAppMap.at(sockfd); }

void Server::_serverLoop() {
	struct epoll_event ev;
	int                clientfd = -1;
	int                nfds;
	struct epoll_event events[MAX_EVENTS];
	char               buffer[8192];
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
				if (_listenClientResponse(buffer, events[i].data.fd))
					continue;
				try {
					RequestMessage  request(buffer);
					ResponseMessage answer = RequestHandler::generateResponse(
					    _getApplicationFromFD(events[i].data.fd).getConfig(), request);
					_sendAnswer(answer.str(), events[i].data.fd);
				} catch (std::exception &e) {
					std::cerr << "Error: " << e.what() << std::endl;
					continue;
				}
			}
		}
	}
}
