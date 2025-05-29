#include "Config.hpp"
#include "utilsParsing.hpp"
#include "utilsSpace.hpp"
#include <algorithm> // pour afficher les tests
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

Config::Config(std::fstream &file) {
	_setDefaultConfig();

	int         i;
	std::string list[] = {"listen", "server_name", "error_page", "client_max_body_size",
	                      "host",   "root",        "index",      "location"};
	void (Config::*functionPointer[])(std::string &, std::fstream &file) = {
	    &Config::_parseListen,    &Config::_parseApplicationName,
	    &Config::_parseErrorPage, &Config::_parseClientMax,
	    &Config::_parseHost,      &Config::_parseRoot,
	    &Config::_parseIndex,     &Config::_parseLocation};

	// mettre dans un check empty
	std::string token = " ";
	while (justSpaces(token)) {
		if (!getline(file, token))
			throw Config::Finished();
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
			throw Config::Exception("Unexpected newline");
		// std::cout << "token = |" << token << "|\n";
		if (token == "}")
			break;
		if (token.empty())
			throw Config::Exception("Missing closing brackets");
		for (i = 0; i < 8; i++) {
			if (token == list[i]) {
				(this->*functionPointer[i])(token, file);
				break;
			}
			if (i == 7)
				throw Config::Exception("Problem parsing file");
		}
		if (i == 8)
			throw Config::Exception("Problem parsing file");
	}

	// check si location / existe bien
	for (i = 0; i < getNumOfLoc(); i++) {
		if (getLocName(i) == "/")
			break;
	}
	if (i == getNumOfLoc())
		throw Config::Exception("No / location");
	_setDefaultLocation();
	// Affichage test
	// std::cout << "TESTTTT = " << getAddress() << " | " << getPort() << std::endl;
	// std::cout << "Listen = |" << _listen << "|" << std::endl;
	// for (std::deque<std::string>::iterator it = _applicationName.begin();
	//      it != _applicationName.end(); it++)
	// 	std::cout << "Config name = " << *it << std::endl;
	// std::cout << "Root = |" << _root << "|" << std::endl;
	// for (std::deque<std::string>::iterator it = _index.begin(); it != _index.end(); it++)
	// 	std::cout << "Index = " << *it << std::endl;
	// std::cout << "Client max = | " << _clientMaxBodySize << " | " << std::endl;
	// std::cout << " Host = | " << _host << " | " << std::endl;
	//
	// std::cout << "Location : " << std::endl;
	// //	std::cout << "Root = |" << _location[0]._root << "|" << std::endl;
	// std::cout << "Root = |" << _location.front().getRoot() << "|" << std::endl;
	// std::cout << "Nmae = |" << _location.front().getName() << "|" << std::endl;
}
Config::Config(const Config &former)
    : _listen(former.getListen()), _address(former.getAddress()), _port(former.getPort()),
      _applicationName(former.getApplicationName()), _errorPages(former.getErrorPages()),
      _clientMaxBodySize(former.getClientMaxBodySize()), _host(former.getHost()),
      _root(former.getRoot()), _index(former.getIndex()), _location(former.getLocation()) {}

Config &Config::operator=(const Config &former) {
	(void)former;
	return *this;
}

Config::~Config() {}

const char *Config::Finished::what() const throw() { return "Finished"; }

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
	_address = _listen.substr(0, _listen.find(':', 0));
	_port = std::atoi(_listen.substr(_listen.find(':', 0) + 1, _listen.length()).c_str());
}

void Config::_parseApplicationName(std::string &str, std::fstream &file) {
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
		_applicationName.push_back(str);
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
	while (start >= 0 && !isspace(str[start]))
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
		_errorPages[code] = page;
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

void Config::_setDefaultErrorPages() {
	std::stringstream pageNameStream;
	std::string       pageName;

	for (int i = 400; i <= 505; ++i) {
		pageNameStream.str("");
		pageNameStream << "website/errorPages/error" << i << ".html";
		pageName = pageNameStream.str();
		_errorPages.insert(std::pair<unsigned short, std::string>(i, pageName));
		if (i == 417)
			i = 499;
	}
}

void Config::_setDefaultConfig() { _setDefaultErrorPages(); }
void Config::_setDefaultLocation() {
	for (std::deque<Location>::iterator it = _location.begin(); it != _location.end(); ++it) {
		if ((*it).getMethods().empty())
			(*it).setDefaultMethods();
		if ((*it).getRoot().empty())
			(*it).setDefaultRoot(getRoot());
	}
}

const std::string             &Config::getListen() const { return _listen; }
const std::string             &Config::getAddress() const { return _address; }
const int                     &Config::getPort() const { return _port; }
const std::deque<std::string> &Config::getApplicationName() const { return _applicationName; }
const std::string             &Config::getErrorPage(int index) const {
    std::map<unsigned short, std::string>::const_iterator it = _errorPages.find(index);
    if (it != _errorPages.end())
        return it->second;
    std::cerr << "Error page " << index << " does not exist" << std::endl;
    throw std::out_of_range("");
}
const std::map<unsigned short, std::string> &Config::getErrorPages() const { return _errorPages; }
const std::string             &Config::getClientMaxBodySize() const { return _clientMaxBodySize; }
const std::string             &Config::getHost() const { return _host; }
const std::string             &Config::getRoot() const { return _root; }
const std::deque<std::string> &Config::getIndex() const { return _index; }
const std::deque<Location>    &Config::getLocation() const { return _location; }

// Location getter , int parameter is the index of the container
const std::string &Config::getLocName(int index) const { return _location.at(index).getName(); }
const std::pair<int, std::string> &Config::getLocRedirection(int index) const {
	return _location.at(index).getRedirection();
}
const std::deque<std::string> &Config::getLocMethods(int index) const {
	return _location.at(index).getMethods();
}
const std::string &Config::getLocRoot(int index) const { return _location.at(index).getRoot(); }
const bool        &Config::getLocAutoindent(int index) const {
    return _location.at(index).getAutoindent();
}

// additionnal getters
int Config::getNumOfLoc() const { return (int)_location.size(); }
