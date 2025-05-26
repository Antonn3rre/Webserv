#include "Server.hpp"
#include "Application.hpp"
#include <iostream>
#include <sstream>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

// Server::Server(void) {
//	Application test;
//	_applicationList.push_back(test);
//	std::cout << _applicationList.size() << std::endl;
//	std::cout << _applicationList[0].getConfig().getPort() << std::endl;
// };

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

Application &Server::getRightApplication(std::pair<std::string, bool> requestHost) {
	std::cout << "Dans right, requestHost = " << requestHost.second << std::endl;
	for (std::vector<Application>::iterator it = _applicationList.begin();
	     it != _applicationList.end(); ++it) {
		for (std::deque<std::string>::const_iterator sit =
		         it->getConfig().getApplicationName().begin();
		     sit != it->getConfig().getApplicationName().end(); ++sit) {
			std::cout << "Valeur sit = " << *sit << std::endl;
			if (*sit == requestHost.first)
				return *it;
		}
	}
	return _applicationList.front();
}

void Server::_initServer(void) {
	_epollfd = epoll_create(MAX_EVENTS);
	for (std::vector<Application>::iterator itServer = _applicationList.begin();
	     itServer != _applicationList.end(); itServer++) {
		itServer->_initApplication(_epollfd);
	}
}

void Server::startServer(void) {
	_initServer();
	std::cout << "TEST" << std::endl;
	_serverLoop();
	// close(_lsockfd); // to put in the signal handler
}

void Server::_sendAnswer(std::string answer, struct epoll_event &event) {
	if (send(event.data.fd, answer.c_str(), answer.length(), MSG_NOSIGNAL) < 0) {
		std::cerr << "Error on write." << std::endl;
		close(event.data.fd);
	}
}

bool Server::_listenClientResponse(struct epoll_event &event, char *buffer) {
	bzero(buffer, 8192);
	if (read(event.data.fd, buffer, 8192) < 0) {
		std::cerr << "Error on read." << std::endl;
		close(event.data.fd);
		return (1);
	}
	std::cout << buffer << std::endl;
	return (0);
}

void Server::_serverLoop() {
	struct epoll_event ev;
	int                clientfd = -1;
	int                nfds;
	struct epoll_event events[MAX_EVENTS];
	char               buffer[8192];

	while (true) {
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, TIME_OUT);

		for (std::vector<Application>::iterator itServer = _applicationList.begin();
		     itServer != _applicationList.end(); itServer++) {
			for (int i = 0; i < nfds; ++i) {
				if (events[i].data.fd == itServer->getLSockFd()) {
					// reinterpreter_cast into `struct sockaddr *` and &clilen for the last
					// parameter (see code in the epoll's man) for accept parameters
					clientfd = accept(itServer->getLSockFd(), NULL, NULL);
					if (clientfd < 0) {
						std::cerr << "Error on accept clients." << std::endl;
						continue;
					}
					ev.events = EPOLLIN | EPOLLET;
					ev.data.fd = clientfd;
					epoll_ctl(_epollfd, EPOLL_CTL_ADD, clientfd, &ev);
				} else {
					if (_listenClientResponse(events[i], buffer))
						continue;
					std::string answer = _buildAnswer();
					_sendAnswer(answer, events[i]);
				}
			}
		}
	}
}

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
	    "<div><img "
	    "src=\"./website/img/IMG_3935.JPG\"></img></div>"
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
