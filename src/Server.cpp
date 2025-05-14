#include "Server.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>

Server::Server(char *configFile) {
	std::fstream file;
	file.open(configFile, std::fstream::in);
	if (!file.is_open()) {
		throw Server::Exception("Problem opening file");
	}

	// manque 1er check de la ligne server {

	std::string str;
	std::string list[] = {"listen", "server_name", "error_page", "client_max_body_size",
	                      "host",   "root",        "index",      "location"};
	void (Server::*functionPointer[])(std::string &, std::fstream &file) = {
	    &Server::_parseListen,    &Server::_parseServerName, &Server::_parseErrorPage,
	    &Server::_parseClientMax, &Server::_parseHost,       &Server::_parseRoot,
	    &Server::_parseIndex,     &Server::_parseLocation};
	while (true) {
		std::getline(file, str, ' '); // /!\ si tab au lieu de space, gros probleme
		if (str.empty())
			break;
		// check ligne avec espaces
		std::cout << "str = " << str << std::endl;
		for (int i = 0; i < 7; i++) {
			if (str == list[i]) {
				(this->*functionPointer[i])(str, file);
				break;
			}
		}
		// ajouter exception si pas reconnu
		// throw Server::Exception("Problem parsing file");
	}
	file.close();

	std::cout << "Listen = " << _listen << std::endl;
	std::cout << "Server name = " << _serverName.front() << std::endl;
	std::cout << "Server name = " << _serverName.back() << std::endl;
	std::cout << "Root = " << _root << std::endl;
	std::cout << "index = " << _index.front() << std::endl;
	std::cout << "index = " << _index.back() << std::endl;
}

Server::Server(const Server &former) { (void)former; }

Server &Server::operator=(const Server &former) {
	(void)former;
	return *this;
}

Server::~Server() {}

Server::Exception::Exception(const std::string &message) : _errorMessage(message) {}

Server::Exception::~Exception() throw() {}

int justSpaces(std::string str) {
	int i = 0;
	if (!str[i])
		return (1);
	while (str[i]) {
		if (str[i] != ' ' && (str[i] < 9 || str[i] > 13))
			return (0);
		i++;
	}
	return (1);
}

std::string trim(std::string &str) {
	size_t start = 0;
	size_t end = str.length() - 1;

	while (start <= end && str[start] == ' ')
		start++;
	while (end > start && str[end] == ' ')
		end--;
	return str.substr(start, end - start + 1);
}

void Server::_parseListen(std::string &str, std::fstream &file) {
	std::getline(file, str);
	// check space
	if (str.empty())
		throw Server::Exception("Problem parse listen");
	_listen = trim(str);
	if (_listen.length() - 1 != _listen.find_first_of(';'))
		throw Server::Exception("Problem parse listen (;)");
}

void Server::_parseServerName(std::string &str, std::fstream &file) {
	int start = 0;
	int end = 0;

	std::getline(file, str);
	// check space
	if (str.empty())
		throw Server::Exception("Problem parse server name");
	if (str.length() - 1 != str.rfind(';'))
		throw Server::Exception("Problem parse server name (;)");
	while (str[end] != ';') {
		while (str[start] == ' ')
			start++;
		end = start;
		while (str[end] != ' ' && str[end] != ';')
			end++;
		if (end == start)
			break;
		_serverName.push_back(str.substr(start, end - start));
		start = end;
	}
}

void Server::_parseErrorPage(std::string &str, std::fstream &file) {
	(void)str;
	(void)file;
}

void Server::_parseClientMax(std::string &str, std::fstream &file) {
	std::getline(file, str);
	// check space
	if (str.empty())
		throw Server::Exception("Problem parse client max body size");
	_clientMaxBodySize = trim(str);
}

void Server::_parseHost(std::string &str, std::fstream &file) {
	std::getline(file, str);
	// check space
	if (str.empty())
		throw Server::Exception("Problem parse host");
	_host = trim(str);
}

void Server::_parseRoot(std::string &str, std::fstream &file) {
	std::getline(file, str);
	// check space
	if (str.empty())
		throw Server::Exception("Problem parse root");
	_root = trim(str);
	if (_root.length() - 1 != _root.rfind(';'))
		throw Server::Exception("Problem parse root (;)");
}

void Server::_parseIndex(std::string &str, std::fstream &file) {
	int start = 0;
	int end = 0;

	std::getline(file, str);
	// check space
	if (str.empty())
		throw Server::Exception("Problem parse index");
	if (str.length() - 1 != str.rfind(';'))
		throw Server::Exception("Problem parse index (;)");
	while (str[end] != ';') {
		while (str[start] == ' ')
			start++;
		end = start;
		while (str[end] != ' ' && str[end] != ';')
			end++;
		if (end == start)
			break;
		_index.push_back(str.substr(start, end - start));
		start = end;
	}
}

void Server::_parseLocation(std::string &str, std::fstream &file) {
	(void)str;
	(void)file;
}
