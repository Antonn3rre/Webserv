#include "Config.hpp"
#include "utilsParsing.hpp"
#include "utilsSpace.hpp"
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

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

	// mettre dans un check empty
	std::string token = " ";
	while (justSpaces(token)) {
		if (!getline(file, token))
			throw Config::Exception("Empty config file");
	}

	// mettre dans un check first line
	std::istringstream iss(token);
	iss >> token;
	if (token != "server{") {
		if (token != "server" || !(iss >> token) || token != "{" || iss >> token)
			throw Config::Exception("Server line wrong");
	}

	// mettre dans un parse body
	while (true) {
		token = readToken(file);
		if (token.empty())
			throw Config::Exception("Erreur : saut de ligne inattendu dans un token !");
		// std::cout << "token = |" << token << "|\n";
		if (token == "}") {
			if (!getline(file, token) || justSpaces(token))
				break;
			throw Config::Exception("closing brackets too soon");
		}
		if (token.empty())
			throw Config::Exception("Missing closing brackets");
		for (int i = 0; i < 8; i++) {
			if (token == list[i]) {
				(this->*functionPointer[i])(token, file);
				break;
			}
			if (i == 7) {
				std::cout << "problem = " << token << std::endl;
				throw Config::Exception("Problem parsing file");
			}
		}
	}
	file.close();
	/*
	    // Affichage test
	    std::cout << "Listen = |" << _listen << "|" << std::endl;
	    for (std::deque<std::string>::iterator it = _serverName.begin(); it != _serverName.end();
	   it++) std::cout << "Config name = " << *it << std::endl; std::cout << "Root = |" << _root <<
	   "|" << std::endl; for (std::deque<std::string>::iterator it = _index.begin(); it !=
	   _index.end(); it++) std::cout << "Index = " << *it << std::endl; std::cout << "Client max =
	   |" << _clientMaxBodySize << "|" << std::endl; std::cout << "Host = |" << _host << "|" <<
	   std::endl;

	    std::cout << "Location : " << std::endl;
	    //	std::cout << "Root = |" << _location[0]._root << "|" << std::endl;
	    std::cout << "Root = |" << _location.front()._root << "|" << std::endl;
	    std::cout << "Nmae = |" << _location.front()._name << "|" << std::endl;
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

void Config::_parseListen(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse listen");
	_listen = trim(str);
	if (_listen.length() - 1 != _listen.find_first_of(';') || _listen.length() == 1)
		throw Config::Exception("Problem parse listen (;)");
	_listen = _listen.substr(0, _listen.length() - 1);
	_listen = trim(_listen);
}

void Config::_parseServerName(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse server name");
	str = trim(str);
	if (str.length() - 1 != str.rfind(';'))
		throw Config::Exception("Problem parse server name (;)");
	str.erase(str.length() - 1);
	if (str.empty())
		throw Config::Exception("Problem parse server name");

	std::istringstream iss(str);
	while (iss >> str) {
		_serverName.push_back(str);
	}
}

void Config::_parseErrorPage(std::string &str, std::fstream &file) {
	std::string page;

	int start;
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse error page");
	str = trim(str);
	if (str.empty() || str[str.length() - 1] != ';')
		throw Config::Exception("Problem parse error page (;)");
	str.erase(str.length() - 1);
	if (str.empty())
		throw Config::Exception("Problem parse error page");

	str = trim(str);
	start = (int)str.length() - 1;
	while (start >= 0 && !isSpace(str[start]))
		start--;
	if (start == -1)
		throw Config::Exception("Problem parse error page");
	page = str.substr(start + 1, str.length() - start);

	str = str.substr(0, start);
	std::istringstream iss(str);
	std::string        token;

	while (iss >> token) {
		std::istringstream issNum(token);
		int                code;
		if (!(issNum >> code) || code < 100 || code > 599)
			throw Config::Exception("Problem parsing error page : wrong error code");
		_errorPage[code] = page;
		//    std::cout << "code = " << code << " et erorpage[code] = " << _errorPage[code] <<
		//    std::endl;
	}
}

void Config::_parseClientMax(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse client max body size");
	_clientMaxBodySize = trim(str);
	if (_clientMaxBodySize.length() - 1 != _clientMaxBodySize.find_first_of(';') ||
	    _clientMaxBodySize.length() == 1)
		throw Config::Exception("Problem parse client max body size (;)");
	_clientMaxBodySize = _clientMaxBodySize.substr(0, _clientMaxBodySize.length() - 1);
	_clientMaxBodySize = trim(_clientMaxBodySize);
}

void Config::_parseHost(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse host");
	_host = trim(str);
	if (_host.length() - 1 != _host.find_first_of(';') || _host.length() == 1)
		throw Config::Exception("Problem parse host (;)");
	_host = _host.substr(0, _host.length() - 1);
	_host = trim(_host);
}

void Config::_parseRoot(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse root");
	_root = trim(str);
	if (_root.length() - 1 != _root.rfind(';'))
		throw Config::Exception("Problem parse root (;)");
	_root = _root.substr(0, _root.length() - 1);
	_root = trim(_root);
}

void Config::_parseIndex(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse index");
	str = trim(str);
	if (str.length() - 1 != str.rfind(';'))
		throw Config::Exception("Problem parse index (;)");
	str.erase(str.length() - 1);
	if (str.empty())
		throw Config::Exception("Problem parse index");

	std::istringstream iss(str);
	while (iss >> str) {
		_index.push_back(str);
	}
}

void Config::_parseLocation(std::string &str, std::fstream &file) {
	try {
		_location.push_back(Location(str, file));
	} catch (Location::Exception &e) {
		throw Config::Exception("Problem parse location");
	}
}

std::string             Config::getListen() const { return _listen; }
std::deque<std::string> Config::getServerName() const { return _serverName; }
std::string             Config::getErrorPage(int index) const { return _errorPage.at(index); }
std::string             Config::getClientMaxBodySize() const { return _clientMaxBodySize; }
std::string             Config::getHost() const { return _host; }
std::string             Config::getRoot() const { return _root; }
std::deque<std::string> Config::getIndex() const { return _index; }
std::deque<Location>    Config::getLocation() const { return _location; }

// Location getter , int parameter is the index of the container
std::string Config::getLocName(int index) const { return _location.at(index).getName(); }
std::string Config::getLocRedirectionUri(int index, int indexUri) const {
	return _location.at(index).getRedirectionUri(indexUri);
}
std::deque<std::string> Config::getLocMethods(int index) const {
	return _location.at(index).getMethods();
}
std::string Config::getLocRoot(int index) const { return _location.at(index).getRoot(); }
bool Config::getLocAutoindent(int index) const { return _location.at(index).getAutoindent(); }
