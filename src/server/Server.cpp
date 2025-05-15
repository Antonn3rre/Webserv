#include "Server.hpp"
#include "Config.hpp"
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

Server::Server(void) : _config(Config("conf/default.conf")){};

Server::Server(char *configFile) : _config(Config(configFile)){};

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
