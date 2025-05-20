#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include <arpa/inet.h>
#include "ResponseMessage.hpp"
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <strings.h>
#include <sys/epoll.h>
#include <fcntl.h>

Server::Server(void) {};
// Server::Server(void) : _config(Config("conf/default.conf")), _sd(0) {};

// Server::Server(char *configFile) {};
// Server::Server(char *configFile) : _config(Config(configFile)) {};

int setnonblocking(int sock)
{
    int result;
    int flags;

    flags = ::fcntl(sock, F_GETFL, 0);

    if (flags == -1)
    {
        return -1;  // error
    }

    flags |= O_NONBLOCK;

    result = fcntl(sock , F_SETFL , flags);
    return result;
}

void Server::startServer(void) {
	int					sockfd, newsockfd, portno;
	socklen_t			clilen;
	struct sockaddr_in	serv_addr, cli_addr;
	int	n;
	char buffer[1000];

	struct epoll_event ev, events[MAX_EVENTS];
	int nfds;

	newsockfd = -1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		std::cerr << "Error on socket." << std::endl;
		exit (1);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 8080;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		std::cerr << "Error on bind." << std::endl;
		exit (1);
	}
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	int	epollfd = epoll_create(MAX_EVENTS);
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, TIME_OUT);

		for (int i = 0; i < nfds; i++) {
			if (events[i].data.fd == sockfd) {
				newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
				if (newsockfd < 0) {
					std::cerr << "Error on bind." << std::endl;
					exit (1);
				}
				setnonblocking(newsockfd);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = newsockfd;
				epoll_ctl(epollfd, EPOLL_CTL_ADD, newsockfd, &ev);
			} else {
				bzero(buffer, 1000);
				n = read(events[i].data.fd, buffer, 1000);
				if (n < 0) {
					std::cerr << "Error on read." << std::endl;
					exit (1);
				}
				std::string tmp = buildAnswer(129);
				n = write(events[i].data.fd, tmp.c_str(), tmp.length());
				if (n < 0) {
					std::cerr << "Error on write." << strerror(errno) << std::endl;
					exit (1);
				}
			}
			// if (newsockfd != -1)
			// 	close(newsockfd);
		}
	}
	close(sockfd);
	// handleClients();
}

void	Server::handleClients(void) {
}

std::string	Server::buildAnswer(unsigned char i) {
	std::string requestStr = "GET /ip HTTP/1.1\nHost: httpbin.org\n\n{\n\tblabla\n\tasdasd\n}";
	std::string responseStr =
		"HTTP/1.1 200 OK\nDate: Mon, 12 May 2025 16:29:56 GMT\nContent-Type: "
		"application/json\nContent-Length: 31\nConnection: keep-alive\nServer: "
		"gunicorn/19.9.0\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Credentials: "
		"true\n\n{\n\t\"origin\": \"62.210.35.12\"\n} ";
	(void) i;
	// std::stringstream ss;
	// ss.str("");
	// ss << "HTTP/1.1 200 OK\r\n"
	// "Date: Mon, 12 May 2025 16:29:56 GMT\r\n"
	// "Content-Type: text/html\r\n"
	// "Content-Length: 104\r\n"
	// "Connection: keep-alive\r\n"
	// "Server: gunicorn/19.9.0\r\n"
	// "Access-Control-Allow-Origin: *\r\n"
	// "Access-Control-Allow-Credentials: true\r\n"
	// "\r\n"
	// "<!DOCTYPE html>\n"
	// "<html>\n"
	// "<head><title>First webserv</title></head>\n"
	// "<body>\n"
	// "    <p>Hello World.</p>\n"
	// "</body>\n"
	// "</html>\n";

	std::string body =
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<head><title>First webserv</title></head>\r\n"
    "<body>\r\n"
    "    <p>Hello World.</p>\r\n"
    "</body>\r\n"
    "</html>\r\n";

	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\n"
	<< "Date: Mon, 12 May 2025 16:29:56 GMT\r\n"
	<< "Content-Type: text/html\r\n"
	<< "Content-Length: " << body.length() << "\r\n"
	<< "Connection: keep-alive\r\n"
	<< "Server: gunicorn/19.9.0\r\n"
	<< "Access-Control-Allow-Origin: *\r\n"
	<< "Access-Control-Allow-Credentials: true\r\n"
	<< "\r\n"
	<< body;

	ResponseMessage response(ss.str());
    return ss.str();
}

// // Config getter
// const std::string             &Server::getListen() const { return _config.getListen(); }
// const std::deque<std::string> &Server::getServerName() const { return _config.getServerName(); }
// const std::string &Server::getErrorPage(int index) const { return _config.getErrorPage(index); }
// const std::string &Server::getClientMaxBodySize() const { return _config.getClientMaxBodySize(); }
// const std::string &Server::getHost() const { return _config.getHost(); }
// const std::string &Server::getRoot() const { return _config.getRoot(); }
// const std::deque<std::string> &Server::getIndex() const { return _config.getIndex(); }
// const std::deque<Location>    &Server::getLocation() const { return _config.getLocation(); }
//
// // Location getter , int parameter is the index of the container
// const std::string &Server::getLocName(int index) const { return _config.getLocName(index); }
// const std::pair<int, std::string> &Server::getLocRedirection(int index) const {
// 	return _config.getLocRedirection(index);
// }
// const std::deque<std::string> &Server::getLocMethods(int index) const {
// 	return _config.getLocMethods(index);
// }
// const std::string &Server::getLocRoot(int index) const { return _config.getLocRoot(index); }
// const bool &Server::getLocAutoindent(int index) const { return _config.getLocAutoindent(index); }
//
// // additionnal getters
// unsigned int Server::getNumOfLoc() const { return _config.getNumOfLoc(); }
