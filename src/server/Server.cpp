#include "Server.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
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
	_server_addr.sin_port = htons(std::atoi("8080"));
	_server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY: server listens on all available local interfaces

	if (bind(_msocket, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) < 0) {
		std::cerr << "Failed to bind server socket." << std::endl;
		exit(1);
	}

	if (listen(_msocket, 5) < 0) {
		std::cerr << "Failed to listen on server socket." << std::endl;
		exit(1);
	}
	std::cout << "Server started on port: " << "8080" << std::endl;
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
		std::string answer = buildAnswer();
	}
}

void	Server::handleMessage(void) {
	char	message[1024];
	for (unsigned int i = 0; i < _client.clientList.size(); i++) {
		_sd = _client.clientList[i];
		if (FD_ISSET(_sd, &_readfds)) {
			_valread = read(_sd, message, 1024);
			if (_valread == 0) {
				std::cout << "Client disconnected" << std::endl;
			} else {
				std::cout << "Message from the client :" << message << std::endl;
			}
		}
	}
}

std::string	Server::buildAnswer(std::string statuscode, std::string statusmsg, std::map<std::string, std::string> headers, std::string body,std::string mimetype) {
    //TODO
    // status code , status msg , headers , body ko response formate me frame krna hai.
    headers["content-type"] = mimetype;
    headers["content-length"] = body.length();
    std::ostringstream buffer;
    buffer << "HTTP/1.1 " << statuscode << " " << statusmsg << "\r\n";
    for(std::map<std::string, std::string>::iterator x; x != headers.end(); x++){
        buffer << x->first << ": " << x->second << "\r\n";
    }
    buffer << "\r\n" << body;
    return buffer.str();
}
