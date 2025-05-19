#include "Server.hpp"
#include "Config.hpp"
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

Server::Server(void) : _config(Config("conf/default.conf")) {};

Server::Server(char *configFile) : _config(Config(configFile)) {};

void Server::startServer(void) {
	_msocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_msocket < 0) {
		std::cerr << "Failed to create server socket." << std::endl;
		exit(1);
	}
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_port = htons(std::atoi(_config.getHost().c_str()));
	_server_addr.sin_addr.s_addr =
	    htonl(INADDR_ANY); // INADDR_ANY: server listens on all available local interfaces
	if (bind(_msocket, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) < 0) {
		std::cerr << "Failed to bind server socket." << std::endl;
		exit(1);
	}
	if (listen(_msocket, 5) < 0) {
		std::cerr << "Failed to listen on server socket." << std::endl;
		exit(1);
	}
	std::cout << "Server started on port: " << "8080" << std::endl;
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
