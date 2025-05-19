#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "ResponseMessage.hpp"
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <vector>

Server::Server(void) : _sd(0) {};
// Server::Server(void) : _config(Config("conf/default.conf")) {};

// Server::Server(char *configFile) {};
// Server::Server(char *configFile) : _config(Config(configFile)) {};

void Server::startServer(void) {
	_msocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_msocket < 0) {
		std::cerr << "Failed to create server socket." << std::endl;
		exit(1);
	}
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons(std::atoi("4342"));
	_server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY: server listens on all available local interfaces

	if (bind(_msocket, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) < 0) {
		std::cerr << "Failed to bind server socket." << std::endl;
		exit(1);
	}

	if (listen(_msocket, 5) < 0) {
		std::cerr << "Failed to listen on server socket." << std::endl;
		exit(1);
	}
	std::cout << "Server started on port: " << "4342" << std::endl;
	handleClient();
}

void	Server::handleClient(void) {
	while (true) {
		std::cout << "Waiting for activity" << std::endl;
		FD_ZERO(&_readfds);
		FD_SET(_msocket, &_readfds);
		_maxfd = _msocket;
		for (std::vector<int>::iterator it = _client.clientList.begin(); it != _client.clientList.end(); it++) {
			FD_SET(_sd, &_readfds);
			if (_sd > _maxfd) _maxfd = _sd;
		}
		if (_sd > _maxfd) _maxfd = _sd;

		_activity = select(_maxfd + 1, &_readfds, NULL, NULL, NULL);
		if (_activity < 0) {
			std::cerr << "Failed on select." << std::endl;
			continue;
		}

		if (FD_ISSET(_msocket, &_readfds)) {
			_client.client_fd = accept(_msocket, (struct sockaddr *) NULL, NULL);
			if (_client.client_fd < 0) {
				std::cerr << "Error on accept." << std::endl;
				continue ;
			}
			_client.clientList.push_back(_client.client_fd);
			std::cout << "New client connected." << std::endl;
		}
		handleMessage();
		std::string tmp = buildAnswer();
		write(_client.client_fd, tmp.c_str(), tmp.size());
		// close(_client.client_fd);
	}
}

void	Server::handleMessage(void) {
	char	message[1024];
	for (unsigned int i = 0; i < _client.clientList.size(); i++) {
		_sd = _client.clientList[i];
		if (FD_ISSET(_sd, &_readfds)) {
			_valread = read(_sd, message, 1024);
			
			if (_valread <= 0) {
				std::cout << "Client disconnected" << std::endl;
				close(_sd);
				_client.clientList.erase(_client.clientList.begin() + i);
			} else {
				std::cout << "Message from the client :" << message << std::endl;
			}
		}
	}
}

std::string	Server::buildAnswer() {
	std::string requestStr = "GET /ip HTTP/1.1\nHost: httpbin.org\n\n{\n\tblabla\n\tasdasd\n}";
	std::string responseStr =
		"HTTP/1.1 200 OK\nDate: Mon, 12 May 2025 16:29:56 GMT\nContent-Type: "
		"application/json\nContent-Length: 31\nConnection: keep-alive\nServer: "
		"gunicorn/19.9.0\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Credentials: "
		"true\n\n{\n\t\"origin\": \"62.210.35.12\"\n} ";

	std::string responseHtml =
	"HTTP/1.1 200 OK\n"
	"Date: Mon, 12 May 2025 16:29:56 GMT\n"
	"Content-Type: text/html\n"
	"Content-Length: 102\n"
	"Connection: keep-alive\n"
	"Server: gunicorn/19.9.0\n"
	"Access-Control-Allow-Origin: *\n"
	"Access-Control-Allow-Credentials: true\n"
	"\n"
	"<!DOCTYPE html>\n"
	"<html>\n"
	"<head><title>First webserv</title></head>\n"
	"<body>\n"
	"    <p>Hello World.</p>\n"
	"</body>\n"
	"</html>\n";
	ResponseMessage response(responseHtml);
    return response.str();
}

// Config getter
const std::string             &Server::getListen() const { return _config.getListen(); }
const std::deque<std::string> &Server::getServerName() const { return _config.getServerName(); }
const std::string &Server::getErrorPage(int index) const { return _config.getErrorPage(index); }
const std::string &Server::getClientMaxBodySize() const { return _config.getClientMaxBodySize(); }
const std::string &Server::getHost() const { return _config.getHost(); }
const std::string &Server::getRoot() const { return _config.getRoot(); }
const std::deque<std::string> &Server::getIndex() const { return _config.getIndex(); }
const std::deque<Location>    &Server::getLocation() const { return _config.getLocation(); }

// Location getter , int parameter is the index of the container
const std::string &Server::getLocName(int index) const { return _config.getLocName(index); }
const std::pair<int, std::string> &Server::getLocRedirection(int index) const {
	return _config.getLocRedirection(index);
}
const std::deque<std::string> &Server::getLocMethods(int index) const {
	return _config.getLocMethods(index);
}
const std::string &Server::getLocRoot(int index) const { return _config.getLocRoot(index); }
const bool &Server::getLocAutoindent(int index) const { return _config.getLocAutoindent(index); }

// additionnal getters
unsigned int Server::getNumOfLoc() const { return _config.getNumOfLoc(); }
