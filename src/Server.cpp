#include "Server.hpp"

Server::Server(char *configFile)
{
	std::fstream file;
	file.open(configFile, std::fstream::in);
	if (!file.is_open())
	{
		throw Server::FileProblemException("Problem opening file");
	}

	file.close();
}

Server::Server(const Server &former)
{
	(void)former;
}

Server &Server::operator=(const Server &former)
{
	(void)former;
	return *this;
}

Server::~Server() {}

Server::FileProblemException::FileProblemException(const std::string &message) : _errorMessage(message) {}

Server::FileProblemException::~FileProblemException() throw() {}
