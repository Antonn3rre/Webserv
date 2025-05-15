#include "Server.hpp"
#include "Config.hpp"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <stdlib.h>
#include <string>

Server::Server(void) : _client_addr_size(sizeof(_client_addr)) {};
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
	_client_socket_fd = accept(_msocket, (struct sockaddr *)&_client_socket_fd, &_client_addr_size);
	if (_client_socket_fd < 0) {
		std::cerr << "Failed to accept client request." << std::endl;
		exit(1);
	}
}
