#include "Config.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>

Config::Config(char *configFile) {
	std::fstream file;
	file.open(configFile, std::fstream::in);
	if (!file.is_open()) {
		throw Config::Exception("Problem opening file");
	}

	// manque 1er check de la ligne server {

	std::string str;
	std::string list[] = {"listen", "server_name", "error_page", "client_max_body_size",
	                      "host",   "root",        "index",      "location"};
	void (Config::*functionPointer[])(std::string &, std::fstream &file) = {
	    &Config::_parseListen,    &Config::_parseServerName, &Config::_parseErrorPage,
	    &Config::_parseClientMax, &Config::_parseHost,       &Config::_parseRoot,
	    &Config::_parseIndex,     &Config::_parseLocation};
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

Config::Config(const Config &former) { (void)former; }

Config &Config::operator=(const Config &former) {
	(void)former;
	return *this;
}

Config::~Config() {}

Config::Exception::Exception(const std::string &message) : _errorMessage(message) {}

Config::Exception::~Exception() throw() {}

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

void Config::_parseListen(std::string &str, std::fstream &file) {
	std::getline(file, str);
	// check space
	if (str.empty())
		throw Config::Exception("Problem parse listen");
	_listen = trim(str);
	if (_listen.length() - 1 != _listen.find_first_of(';'))
		throw Config::Exception("Problem parse listen (;)");
}

void Config::_parseServerName(std::string &str, std::fstream &file) {
	int start = 0;
	int end = 0;

	std::getline(file, str);
	// check space
	if (str.empty())
		throw Config::Exception("Problem parse server name");
	if (str.length() - 1 != str.rfind(';'))
		throw Config::Exception("Problem parse server name (;)");
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

void Config::_parseErrorPage(std::string &str, std::fstream &file) {
	(void)str;
	(void)file;
}

void Config::_parseClientMax(std::string &str, std::fstream &file) {
	std::getline(file, str);
	// check space
	if (str.empty())
		throw Config::Exception("Problem parse client max body size");
	_clientMaxBodySize = trim(str);
}

void Config::_parseHost(std::string &str, std::fstream &file) {
	std::getline(file, str);
	// check space
	if (str.empty())
		throw Config::Exception("Problem parse host");
	_host = trim(str);
}

void Config::_parseRoot(std::string &str, std::fstream &file) {
	std::getline(file, str);
	// check space
	if (str.empty())
		throw Config::Exception("Problem parse root");
	_root = trim(str);
	if (_root.length() - 1 != _root.rfind(';'))
		throw Config::Exception("Problem parse root (;)");
}

void Config::_parseIndex(std::string &str, std::fstream &file) {
	int start = 0;
	int end = 0;

	std::getline(file, str);
	// check space
	if (str.empty())
		throw Config::Exception("Problem parse index");
	if (str.length() - 1 != str.rfind(';'))
		throw Config::Exception("Problem parse index (;)");
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

void Config::_parseLocation(std::string &str, std::fstream &file) {
	(void)str;
	(void)file;
}
