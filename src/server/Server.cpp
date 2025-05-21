#include "Server.hpp"
#include "Config.hpp"
#include "ResponseMessage.hpp"
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
#include <sstream>
#include <stdlib.h>
#include <string>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Server::Server(void) : _config(Config("conf/defaultWithoutCommentaries.conf")) {};

// Server::Server(char *configFile) : _config(Config(configFile)) {};

int setnonblocking(int sock) {
	int result;
	int flags;

	flags = fcntl(sock, F_GETFL, 0);

	if (flags == -1) {
		return -1; // error
	}

	flags |= O_NONBLOCK;

	result = fcntl(sock, F_SETFL, flags);
	return result;
}

void Server::startServer(void) {
	int                clientfd = -1;
	socklen_t          clilen;
	struct sockaddr_in servAddr;
	struct sockaddr_in cliAddr;
	ssize_t            bytes;
	char               buffer[100000];
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];
	int                nfds;

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
	servAddr.sin_port = htons(_config.getPort());
	if (bind(_lsockfd, reinterpret_cast<struct sockaddr *>(&servAddr), sizeof(servAddr)) < 0) {
		std::cerr << "Error on bind listen." << std::endl;
		exit(1);
	}
	listen(_lsockfd, 5);
	clilen = sizeof(cliAddr);

	_epollfd = epoll_create(MAX_EVENTS);
	ev.events = EPOLLIN;
	ev.data.fd = _lsockfd;
	epoll_ctl(_epollfd, EPOLL_CTL_ADD, _lsockfd, &ev);

	std::cout << "Server launch on port: " << _config.getPort() << std::endl;
	for (;;) {
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, TIME_OUT);

		for (int i = 0; i < nfds; ++i) {
			if (events[i].data.fd == _lsockfd) {
				clientfd = accept(_lsockfd, reinterpret_cast<struct sockaddr *>(&cliAddr), &clilen);
				if (clientfd < 0) {
					std::cerr << "Error on bind clients." << std::endl;
					continue;
				}
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = clientfd;
				setnonblocking(ev.data.fd);
				epoll_ctl(_epollfd, EPOLL_CTL_ADD, clientfd, &ev);
			} else {
				bzero(buffer, 100000);
				bytes = read(events[i].data.fd, buffer, 100000);
				if (bytes < 0) {
					std::cerr << "Error on read." << std::endl;
					close(events[i].data.fd);
					continue;
				}
				std::cout << buffer << std::endl;
				std::string tmp = _buildAnswer();
				bytes = send(events[i].data.fd, tmp.c_str(), tmp.length(), MSG_NOSIGNAL);
				if (bytes < 0) {
					std::cerr << "Error on write => " << strerror(errno) << std::endl;
					close(events[i].data.fd);
				}
			}
		}
	}
	close(_lsockfd);
}

void Server::handleClients(void) {}

std::string Server::_buildAnswer() {
	std::string body =
	    "<!DOCTYPE html>\r\n"
	    "<html>\r\n"
	    "<head><title>First webserv</title></head>\r\n"
	    "<body>\r\n"
	    "<div align=\"center\">\r\n"
	    "<img "
	    "src=\"https://remeng.rosselcdn.net/sites/default/files/dpistyles_v2/rem_16_9_1124w/2020/"
	    "09/30/node_194669/12124212/public/2020/09/30/"
	    "B9724766829Z.1_20200930170647_000%2BG3EGPBHMU.1-0.jpg?itok=7_rsY6Fj1601564062\" "
	    "width=\"1200\" height=\"800\" />\r\n"
	    "<a href=\"https://www.google.com/"
	    "url?sa=i&url=https%3A%2F%2Fwww.lardennais.fr%2Fid194669%2Farticle%2F2020-09-30%2Fles-"
	    "punaises-de-lit-lui-ont-fait-vivre-un-enfer-charleville-mezieres&psig=AOvVaw1InOT-"
	    "kYF5steCdhBc6F-7&ust=1747918462193000&source=images&cd=vfe&opi=89978449&ved="
	    "0CBcQjhxqFwoTCLCDguvNtI0DFQAAAAAdAAAAABAE\" target=_blank>\r\n"
	    "<h1> A Charleville - Mézières, les punaises de lit lui ont fait vivre un enfer "
	    "</h1></a>\r\n"
	    "<img "
	    "src=\"https://media1.tenor.com/m/WckcGq81cjYAAAAd/herv%C3%A9-regnier-tiktok.gif\"></img>"
	    "</div>\r\n"
	    "</body>\r\n"
	    "</html>\r\n";

	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\n"
	   << "Date: Mon, 12 May 2025 16:29:56 GMT\r\n"
	   << "Content-Type: text/html; charset=utf-8\r\n"
	   << "Content-Length: " << body.length() << "\r\n"
	   << "Connection: keep-alive\r\n"
	   << "Server: gunicorn/19.9.0\r\n"
	   << "Access-Control-Allow-Origin: *\r\n"
	   << "Access-Control-Allow-Credentials: true\r\n"
	   << "\r\n"
	   << body;

	// ResponseMessage response(ss.str());
	return ss.str();
}
