#include "Server.hpp"
#include "utilsSpace.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>

#include <algorithm> // pour afficher les tests

std::string readToken(std::fstream& file) {
    std::string token;
    char c;
    while (file.get(c)) {
        if (c == '\n') {
            throw Server::Exception("Erreur : saut de ligne inattendu dans un token !");
        }
        if (isSpace(c)) {
            break; // fin du token
        }
        token += c;
    }

    return token;
}

Server::Server(char *configFile) {
	std::fstream file;
	file.open(configFile, std::fstream::in);
	if (!file.is_open()) {
		throw Server::Exception("Problem opening file");
	}

	std::string list[] = {"listen", "server_name", "error_page", "client_max_body_size",
	                      "host",   "root",        "index",      "location"};
	void (Server::*functionPointer[])(std::string &, std::fstream &file) = {
	    &Server::_parseListen,    &Server::_parseServerName, &Server::_parseErrorPage,
	    &Server::_parseClientMax, &Server::_parseHost,       &Server::_parseRoot,
	    &Server::_parseIndex,     &Server::_parseLocation};

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
        throw Server::Exception("Problem parsing file");
		}
    token = ' ';
	}
	file.close();


  // Affichage test
/*
	std::cout << "Listen = |" << _listen << "|" << std::endl;
  for(std::deque<std::string>::iterator it = _serverName.begin() ; it != _serverName.end() ; it++)
	  std::cout << "Server name = " << *it << std::endl;
	std::cout << "Root = |" << _root << "|" << std::endl;
  for(std::deque<std::string>::iterator it = _index.begin() ; it != _index.end() ; it++)
	  std::cout << "Index = " << *it << std::endl;
	std::cout << "Client max = |" << _clientMaxBodySize << "|" << std::endl;
	std::cout << "Host = |" << _host << "|" << std::endl;
*/
}

Server::Server(const Server &former) { (void)former; }

Server &Server::operator=(const Server &former) {
	(void)former;
return *this;
}

Server::~Server() {}

Server::Exception::Exception(const std::string &message) : _errorMessage(message) {}

Server::Exception::~Exception() throw() {}

std::string trim(std::string &str) {
	size_t start = 0;
	size_t end = str.length() - 1;

	while (start <= end && isSpace(str[start]))
		start++;
	while (end > start && isSpace(str[end]))
		end--;
	return str.substr(start, end - start + 1);
}

void Server::_parseListen(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Server::Exception("Problem parse listen");
	_listen = trim(str);
	if (_listen.length() - 1 != _listen.find_first_of(';') || _listen.length() == 1)
		throw Server::Exception("Problem parse listen (;)");
  _listen = _listen.substr(0, _listen.length() -1);
  _listen = trim(_listen);
}

void Server::_parseServerName(std::string &str, std::fstream &file) {
	int start = 0;
	int end = 0;

	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Server::Exception("Problem parse server name");
	if (str.length() - 1 != str.rfind(';'))
		throw Server::Exception("Problem parse server name (;)");
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

void Server::_parseErrorPage(std::string &str, std::fstream &file) {
	(void)str;
	(void)file;
}

void Server::_parseClientMax(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Server::Exception("Problem parse client max body size");
	_clientMaxBodySize = trim(str);
	if (_clientMaxBodySize.length() - 1 != _clientMaxBodySize.find_first_of(';') || _clientMaxBodySize.length() == 1)
		throw Server::Exception("Problem parse client max body size (;)");
  _clientMaxBodySize = _clientMaxBodySize.substr(0, _clientMaxBodySize.length() -1);
  _clientMaxBodySize = trim(_clientMaxBodySize);
}

void Server::_parseHost(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Server::Exception("Problem parse host");
	_host = trim(str);
	if (_host.length() - 1 != _host.find_first_of(';') || _host.length() == 1)
		throw Server::Exception("Problem parse host (;)");
  _host = _host.substr(0, _host.length() -1);
  _host = trim(_host);
}

void Server::_parseRoot(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Server::Exception("Problem parse root");
	_root = trim(str);
	if (_root.length() - 1 != _root.rfind(';'))
		throw Server::Exception("Problem parse root (;)");
  _root = _root.substr(0, _root.length() -1);
  _root = trim(_root);
}

void Server::_parseIndex(std::string &str, std::fstream &file) {
	int start = 0;
	int end = 0;

	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Server::Exception("Problem parse index");
	if (str.length() - 1 != str.rfind(';'))
		throw Server::Exception("Problem parse index (;)");
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

void Server::_parseLocation(std::string &str, std::fstream &file) {
	(void)str;
	(void)file;
}
