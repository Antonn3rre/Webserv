#include "Server.hpp"
#include "Application.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <exception>
#include <iostream>
#include <sstream>
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
	// close(_lsockfd); // to put in the signal handler
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
		return 1;
	}
	return false;
}

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

					// std::cout << " --- ANSWER --- \n"
					//           << answer.getHeaderValue("Content-Type").first << std::endl;
					_sendAnswer(answer.str(), events[i].data.fd);
				} catch (std::exception &e) {
					std::cerr << "Error: " << e.what() << std::endl;
					continue;
				}
			}
		}
	}
}

Application &Server::_getApplicationFromFD(int sockfd) const { return *_clientAppMap.at(sockfd); }

std::string Server::_buildAnswer(int i) {
	std::stringstream body;
	body << "<!DOCTYPE html>\r\n"
	     << "<html>\r\n"
	     << "<head><title>First webserv</title></head>\r\n"
	     << "<body>\r\n"
	     << "<h1> _lsockfd = " << i << "</h1>\r\n"
	     << "<div align=\"center\">\r\n"
	     << "<img "
	     << "src=\"https://remeng.rosselcdn.net/sites/default/files/dpistyles_v2/rem_16_9_1124w/"
	     << "2020/" << "09/30/node_194669/12124212/public/2020/09/30/"
	     << "B9724766829Z.1_20200930170647_000%2BG3EGPBHMU.1-0.jpg?itok=7_rsY6Fj1601564062\" "
	     << "width=\"1200\" height=\"800\" />\r\n"
	     << "<a href=\"https://www.google.com/"
	     << "url?sa=i&url=https%3A%2F%2Fwww.lardennais.fr%2Fid194669%2Farticle%2F2020-09-30%"
	        "2Fles-"
	     << "punaises-de-lit-lui-ont-fait-vivre-un-enfer-charleville-mezieres&psig=AOvVaw1InOT-"
	     << "kYF5steCdhBc6F-7&ust=1747918462193000&source=images&cd=vfe&opi=89978449&ved="
	     << "0CBcQjhxqFwoTCLCDguvNtI0DFQAAAAAdAAAAABAE\" target=_blank>\r\n"
	     << "<h1> A Charleville - Mézières, les punaises de lit lui ont fait vivre un enfer "
	     << "</h1></a>\r\n"
	     << "<img "
	     << "src=\"https://media1.tenor.com/m/WckcGq81cjYAAAAd/"
	        "herv%C3%A9-regnier-tiktok.gif\"></"
	     << "img>" << "</div>\r\n"
	     << "<div><a href=\"/cgi-bin\"><button type=\"button\" id=\"get-btn\"> Hello world! "
	        "</button></a></div>"
	     << "<script>" << "const button = document.getElementById('get-btn');"
	     << "button.addEventListener('click', async () => {" << "  try {"
	     << "    const response = await fetch('/cgi-bin');"
	     << "    const text = await response.text();" << "    console.log('Réponse reçue:', text);"
	     << "  } catch (err) {" << "    console.error('Erreur :', err);" << "  }" << "});"
	     << "</script>"

	     << "</body>\r\n"
	     << "</html>\r\n";

	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\n"
	   << "Date: Mon, 12 May 2025 16:29:56 GMT\r\n"
	   << "Content-Type: text/html; charset=utf-8\r\n"
	   << "Content-Length: " << body.str().length() << "\r\n"
	   << "Connection: keep-alive\r\n"
	   << "Server: gunicorn/19.9.0\r\n"
	   << "Access-Control-Allow-Origin: *\r\n"
	   << "Access-Control-Allow-Credentials: true\r\n"
	   << "\r\n"
	   << body.str();

	// ResponseMessage response(ss.str());
	return ss.str();
}
