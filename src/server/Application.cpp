#include "Application.hpp"
#include "Config.hpp"
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
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Application::Application(std::fstream &file) : _config(Config(file)){};

Application::Application(const Application &former) : _config(former.getConfig()) {
	_lsockfd = former.getLSockFd();
}

bool Application::initApplication(int epollfd) {
	struct epoll_event ev;
	struct sockaddr_in servAddr;
	_lsockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_lsockfd == -1) {
		std::cerr << "Error on socket." << std::endl;
		return (false);
	}

	// allow the socket to be reusable
	int on = 1;
	setsockopt(_lsockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(_config.getPort());
	if (bind(_lsockfd, reinterpret_cast<struct sockaddr *>(&servAddr), sizeof(servAddr)) == -1) {
		std::cerr << "Error on the primary bind." << std::endl;
		return (false);
	}
	if (listen(_lsockfd, 5) == -1) {
		std::cerr << "Error on listen." << std::endl;
		return (false);
	}

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = _lsockfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, _lsockfd, &ev) == -1) {
		std::cerr << "Error on epoll_ctl." << std::endl;
		return (false);
	}
	_printAtLaunch();
	return (true);
}

void Application::close() const { ::close(_lsockfd); }

void Application::_printAtLaunch(void) {
	std::cout << "Server launch at this address: http://" << _config.getAddress() << ":"
	          << _config.getPort() << "/" << std::endl;
}

// Config getter
const Config &Application::getConfig(void) const { return _config; };

const int &Application::getLSockFd(void) const { return _lsockfd; };
