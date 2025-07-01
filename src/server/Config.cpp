#include "Config.hpp"
#include "utilsParsing.hpp"
#include "utilsSpace.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <utility>

Config::Config(std::fstream &file)
    : _address(""), _port(-1), _redirection(std::pair<int, std::string>(-1, "")) {
	_setDefaultConfig();

	std::string list[] = {"listen", "server_name", "error_page", "client_max_body_size",
	                      "root",   "index",       "location",   "return"};
	void (Config::*functionPointer[])(std::string &, std::fstream &file) = {
	    &Config::_parseListen,    &Config::_parseApplicationName,
	    &Config::_parseErrorPage, &Config::_parseClientMaxSizeBody,
	    &Config::_parseRoot,      &Config::_parseIndex,
	    &Config::_parseLocation,  &Config::_parseRedirection};

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

	int i;
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
		}
		if (i == 9)
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
	if (getAddress().empty())
		throw Config::Exception("Missing listen line");
	if (getPort() == -1)
		throw Config::Exception("Missing listen line");
}
Config::Config(const Config &former)
    : _address(former.getAddress()), _port(former.getPort()),
      _applicationName(former.getApplicationName()), _errorPages(former.getErrorPages()),
      _clientMaxBodySize(former.getClientMaxBodySize()), _root(former.getRoot()),
      _index(former.getIndex()), _locations(former.getLocations()),
      _redirection(former.getRedirection()) {}

Config &Config::operator=(const Config &former) {
	if (this != &former) {
		_address = former.getAddress();
		_port = former.getPort();
		_applicationName = former.getApplicationName();
		_errorPages = former.getErrorPages();
		_clientMaxBodySize = former.getClientMaxBodySize();
		_root = former.getRoot();
		_index = former.getIndex();
		_locations = former.getLocations();
		_redirection = former.getRedirection();
	}
	return *this;
}

Config::~Config() {}

const char *Config::Finished::what() const throw() { return "Finished"; }

Config::Exception::Exception(const std::string &message) : _errorMessage(message) {}

Config::Exception::~Exception() throw() {}

void Config::_parseListen(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse listen, empty");
	std::string listen = trim(str);
	if (listen.length() - 1 != listen.find(';'))
		throw Config::Exception("Problem parse listen, missing or misplaced ';'");
	listen.resize(listen.length() - 1);
	listen = trim(listen);
	size_t semicolonPos = listen.find(':');
	if (semicolonPos >= listen.length() - 1)
		throw Config::Exception("Problem parse listen");
	try {
		_address = listen.substr(0, semicolonPos);
		_port =
		    std::atoi(listen.substr(semicolonPos + 1, listen.length() - semicolonPos - 1).c_str());
	} catch (const std::exception &e) {
		std::cerr << "bite" << std::endl;
	}
}

void Config::_parseApplicationName(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse server name, empty");
	str = trim(str);
	if (str.length() - 1 != str.find(';'))
		throw Config::Exception("Problem parse server name, missing or misplaced ';'");
	str.resize(str.length() - 1);
	if (str.empty())
		throw Config::Exception("Problem parse server name");

	std::istringstream iss(str);
	while (iss >> str) {
		_applicationName.push_back(str);
	}
}

void Config::_parseErrorPage(std::string &str, std::fstream &file) {
	std::string page;

	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse error page");
	str = trim(str);
	if (str.length() - 1 != str.find(';'))
		throw Config::Exception("Problem parse error page (;)");
	str.resize(str.length() - 1);
	if (str.empty())
		throw Config::Exception("Problem parse error page");

	str = trim(str);
	int start = (int)str.length() - 1;
	while (start >= 0 && !isspace(str[start]))
		start--;
	if (start == -1)
		throw Config::Exception("Problem parse error page");
	try {
		page = str.substr(start + 1, str.length() - start);

		str = str.substr(0, start);
	} catch (const std::exception &e) {
		std::cerr << "zob" << std::endl;
	}
	std::istringstream iss(str);
	std::string        token;

	while (iss >> token) {
		std::istringstream issNum(token);
		int                code;
		if (!(issNum >> code) || code < 100 || code > 599)
			throw Config::Exception("Problem parsing error page : wrong error code");
		_errorPages[code] = page;
	}
}

void Config::_parseClientMaxSizeBody(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse client max body size");
	str = trim(str);
	if (str.length() - 1 != str.find(';') || str.length() == 1)
		throw Config::Exception("Problem parse client max body size (;)");
	str.resize(str.length() - 1);
	str = trim(str);

	int val = std::atoi(str.c_str());

	std::string multiplierLetter;
	for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
		if (!std::isdigit(*it))
			multiplierLetter += *it;
	}
	if (multiplierLetter.length() > 1 ||
	    (multiplierLetter.length() == 1 &&
	     multiplierLetter.find_first_of("kmgKMG") == std::string::npos))
		throw Config::Exception("client_max_body_size bad format.");
	if (multiplierLetter.empty())
		_clientMaxBodySize = val;
	else if (multiplierLetter == "k" || multiplierLetter == "K")
		_clientMaxBodySize = val * 1000;
	else if (multiplierLetter == "m" || multiplierLetter == "M")
		_clientMaxBodySize = val * 1000000;
	else if (multiplierLetter == "g" || multiplierLetter == "G")
		_clientMaxBodySize = val * 1000000000;
}

void Config::_parseRoot(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse root");
	_root = trim(str);
	if (_root.length() - 1 != _root.find(';'))
		throw Config::Exception("Problem parse root (;)");

	_root.resize(_root.length() - 1);
	_root = trim(_root);
}

void Config::_parseIndex(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Config::Exception("Problem parse index");
	str = trim(str);
	if (str.length() - 1 != str.find(';'))
		throw Config::Exception("Problem parse index (;)");
	str.resize(str.length() - 1);
	if (str.empty())
		throw Config::Exception("Problem parse index");

	std::istringstream iss(str);
	while (iss >> str)
		_index.push_back(str);
}

void Config::_parseLocation(std::string &str, std::fstream &file) {
	try {
		_locations.push_back(Location(str, file, _clientMaxBodySize));
	} catch (Location::Exception &e) {
		throw Config::Exception(e.what());
	}
}

void Config::_parseRedirection(std::string &str, std::fstream &file) {
	if (_redirection.first != -1)
		throw Location::Exception("Problem parse Redirection");
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception("Problem parse Redirection");
	str = trim(str);
	if (str.empty() || str[str.length() - 1] != ';')
		throw Location::Exception("Problem parse Redirection");
	str.erase(str.length() - 1);
	if (str.empty())
		throw Location::Exception("Problem parse Redirection");
	str = trim(str);

	std::istringstream issNum(str);
	if (!(issNum >> _redirection.first) || _redirection.first < 100 || _redirection.first > 599)
		throw Location::Exception("Problem parse Redirection: invalid data");
	if (!(issNum >> _redirection.second))
		_redirection.second = getErrorPage(_redirection.first);
	else if (issNum >> str)
		throw Location::Exception("Problem parse Redirection: invalid data");
}

void Config::_setDefaultErrorPages() {
	std::stringstream pageNameStream;
	std::string       pageName;

	for (unsigned short i = 400; i <= 505; ++i) {
		pageNameStream.str("");
		pageNameStream.clear();
		pageNameStream << "website/errorPages/error" << i << ".html";
		pageName = pageNameStream.str();
		_errorPages[i] = pageName;
		if (i == 417)
			i = 499;
	}
}

void Config::_setDefaultConfig() { _setDefaultErrorPages(); }

void Config::_setDefaultLocation() {
	for (std::vector<Location>::iterator it = _locations.begin(); it != _locations.end(); ++it) {
		if (it->getMethods().empty())
			it->setDefaultMethods();
		if (it->getRoot().empty())
			it->setDefaultRoot(getRoot());
		if (it->getIndex().empty())
			it->setDefaultIndex(getIndex());
		if (it->getRedirection().first == -1)
			it->setDefaultRedirection(getRedirection());
		else if (it->getRedirection().second.empty())
			it->setRedirectionPage(getErrorPage(it->getRedirection().first));
	}
}

const std::string              &Config::getAddress() const { return _address; }
const int                      &Config::getPort() const { return _port; }
const std::vector<std::string> &Config::getApplicationName() const { return _applicationName; }
std::string                     Config::getErrorPage(unsigned short status) const {
    std::map<unsigned short, std::string>::const_iterator it = _errorPages.find(status);

    if (it != _errorPages.end())
        return it->second;
    std::cout << "Error page " << status << " does not exist" << std::endl;
    throw std::out_of_range("");
}
const std::map<unsigned short, std::string> &Config::getErrorPages() const { return _errorPages; }
unsigned long int               Config::getClientMaxBodySize() const { return _clientMaxBodySize; }
const std::string              &Config::getRoot() const { return _root; }
const std::vector<std::string> &Config::getIndex() const { return _index; }
const std::vector<Location>    &Config::getLocations() const { return _locations; }
const std::pair<int, std::string> &Config::getRedirection() const { return _redirection; }

// Location getter , int parameter is the index of the container
const std::string &Config::getLocName(int index) const { return _locations.at(index).getName(); }
const std::pair<int, std::string> &Config::getLocRedirection(int index) const {
	return _locations.at(index).getRedirection();
}
const std::vector<std::string> &Config::getLocMethods(int index) const {
	return _locations.at(index).getMethods();
}
const std::vector<std::string> &Config::getLocIndex(int index) const {
	return _locations.at(index).getIndex();
}

const std::string &Config::getLocRoot(int index) const { return _locations.at(index).getRoot(); }
const bool &Config::getLocAutoindex(int index) const { return _locations.at(index).getAutoindex(); }

// additionnal getters
int Config::getNumOfLoc() const { return (int)_locations.size(); }
