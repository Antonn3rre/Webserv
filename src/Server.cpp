#include "../inc/Server.hpp"

Server::Server( char *configFile ) {
	std::fstream file;
	file.open(configFile, std::fstream::in);
	if (!file.is_open()) {
		throw Server::FileProblemException("Problem opening file");
	}

	file.close();
}

Server::Server(const Server &former) {

}

Server &Server::operator=(const Server &former) {
	return *this;
}

Server::~Server() {}

Server::FileProblemException::FileProblemException(const std::string &message) : _errorMessage(message) {}
