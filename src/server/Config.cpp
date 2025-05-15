#include "Config.hpp"
#include "utilsSpace.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>

Config::Config(const std::string &configFile) {
	std::fstream file;
	file.open(configFile.c_str(), std::fstream::in);
	if (!file.is_open()) {
		throw Config::Exception("Problem opening file");
	}

	std::string list[] = {"listen", "server_name", "error_page", "client_max_body_size",
	                      "host",   "root",        "index",      "location"};
	void (Config::*functionPointer[])(std::string &, std::fstream &file) = {
	    &Config::_parseListen,    &Config::_parseServerName, &Config::_parseErrorPage,
	    &Config::_parseClientMax, &Config::_parseHost,       &Config::_parseRoot,
	    &Config::_parseIndex,     &Config::_parseLocation};

	// manque 1er check de la ligne server {


	std::string token = " ";
	while (true) {
    while (!token.empty() && ((token[0] >= 9 && token[0] <= 13) || token[0] == ' ') )
      token = readToken(file);
    if (token.empty())
			break;
		for (int i = 0; i < 8; i++) {
			if (token == list[i]) {
				(this->*functionPointer[i])(token, file);
				break;
			}
      if (i == 7)
        throw Config::Exception("Problem parsing file");
		}
    token = ' ';
	}
	file.close();


  // Affichage test
/*
	std::cout << "Listen = |" << _listen << "|" << std::endl;
  for(std::deque<std::string>::iterator it = _serverName.begin() ; it != _serverName.end() ; it++)
	  std::cout << "Config name = " << *it << std::endl;
	std::cout << "Root = |" << _root << "|" << std::endl;
  for(std::deque<std::string>::iterator it = _index.begin() ; it != _index.end() ; it++)
	  std::cout << "Index = " << *it << std::endl;
	std::cout << "Client max = |" << _clientMaxBodySize << "|" << std::endl;
	std::cout << "Host = |" << _host << "|" << std::endl;
*/
}

Config::Config(const Config &former) { (void)former; }

Config &Config::operator=(const Config &former) {
	(void)former;
	return *this;
}

Config::~Config() {}

Config::Exception::Exception(const std::string &message) : _errorMessage(message) {}

Config::Exception::~Exception() throw() {}

std::string trim(std::string &str) {
	size_t start = 0;
	size_t end = str.length() - 1;

	while (start <= end && isSpace(str[start]))
		start++;
	while (end > start && isSpace(str[end]))
		end--;
	return str.substr(start, end - start + 1);
}

void Config::_parseListen(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse listen");
	_listen = trim(str);
	if (_listen.length() - 1 != _listen.find_first_of(';') || _listen.length() == 1)
		throw Config::Exception("Problem parse listen (;)");
  _listen = _listen.substr(0, _listen.length() -1);
  _listen = trim(_listen);
}

void Config::_parseServerName(std::string &str, std::fstream &file) {
	int start = 0;
	int end = 0;

	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse server name");
	if (str.length() - 1 != str.rfind(';'))
		throw Config::Exception("Problem parse server name (;)");
	while (str[end] != ';') {
		while (isSpace(str[start]))
			start++;
		end = start;
		while (!isSpace(str[end]) && str[end] != ';')
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
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse client max body size");
	_clientMaxBodySize = trim(str);
	if (_clientMaxBodySize.length() - 1 != _clientMaxBodySize.find_first_of(';') || _clientMaxBodySize.length() == 1)
		throw Config::Exception("Problem parse client max body size (;)");
  _clientMaxBodySize = _clientMaxBodySize.substr(0, _clientMaxBodySize.length() -1);
  _clientMaxBodySize = trim(_clientMaxBodySize);
}

void Config::_parseHost(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse host");
	_host = trim(str);
	if (_host.length() - 1 != _host.find_first_of(';') || _host.length() == 1)
		throw Config::Exception("Problem parse host (;)");
  _host = _host.substr(0, _host.length() -1);
  _host = trim(_host);
}

void Config::_parseRoot(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse root");
	_root = trim(str);
	if (_root.length() - 1 != _root.rfind(';'))
		throw Config::Exception("Problem parse root (;)");
  _root = _root.substr(0, _root.length() -1);
  _root = trim(_root);
}

void Config::_parseIndex(std::string &str, std::fstream &file) {
	int start = 0;
	int end = 0;

	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse index");
	if (str.length() - 1 != str.rfind(';'))
		throw Config::Exception("Problem parse index (;)");
	while (str[end] != ';') {
		while (isSpace(str[start]))
			start++;
		end = start;
		while (!isSpace(str[end]) && str[end] != ';')
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

std::string	Config::getHost(void) const { return _host; };
