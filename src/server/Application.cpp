#include "Application.hpp"
#include "Config.hpp"
#include "HandleRequest.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

Application::Application(void) : _config(Config("conf/defaultWithoutCommentaries.conf")) {};

// Server::Server(char *configFile) : _config(Config(configFile)) {};

void Application::_initApplication(int epollfd) {
	struct epoll_event ev;
	struct sockaddr_in servAddr;
	_lsockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_lsockfd == -1) {
		std::cerr << "Error on socket." << std::endl;
		exit(1);
	}

	// allow the socket to be reusable
	int on = 1;
	setsockopt(_lsockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(8080);
	// servAddr.sin_port = htons(_config.getPort());
	if (bind(_lsockfd, reinterpret_cast<struct sockaddr *>(&servAddr), sizeof(servAddr)) == -1) {
		std::cerr << "Error on the primary bind." << std::endl;
		exit(1);
	}
	if (listen(_lsockfd, 5) == -1) {
		std::cerr << "Error on listen." << std::endl;
		exit(1);
	}

	ev.events = EPOLLIN;
	ev.data.fd = _lsockfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, _lsockfd, &ev) == -1) {
		std::cerr << "Error on epoll_ctl." << std::endl;
		exit(1);
	}
	_printAtLaunch();
}

void Application::_printAtLaunch(void) {
	std::cout << "Server launch at this address: http://" << _config.getAddress() << ":"
	          << _config.getPort() << "/" << std::endl;
}

// Config getter
const Config &Application::getConfig(void) const { return _config; };
const int    &Application::getLSockFd(void) const { return _lsockfd; };
