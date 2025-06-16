#include "Location.hpp"
#include "utilsParsing.hpp"
#include "utilsSpace.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

Location::Location(std::string &token, std::fstream &file, unsigned int clientMaxSizeBody)
    : _redirection(-1, ""), _autoindex(false), _clientMaxBodySize(clientMaxSizeBody) {
	std::string list[] = {"return", "allow_methods", "index",
	                      "root",   "autoindex",     "client_max_body_size"};
	void (Location::*functionPointer[])(std::string &, std::fstream &) = {
	    &Location::_parseRedirection, &Location::_parseMethods, &Location::_parseIndex,
	    &Location::_parseRoot,        &Location::_parseIndent,  &Location::_parseClientMaxSizeBody};

	const unsigned int nbOfMethods = 6;

	getline(file, token);
	std::istringstream iss(token);
	if (!(iss >> token))
		throw Location::Exception("Location construction error");
	_name = token;
	if (_name[0] != '/' || !(iss >> token) || token != "{" || iss >> token)
		throw Location::Exception("Location construction error: bad format");
	while (true) {
		token = readToken(file);
		if (token == "}")
			break;
		if (token.empty())
			throw Location::Exception("Location construction error: empty");
		unsigned int i = 0;
		for (; i < nbOfMethods; i++) {
			if (token == list[i]) {
				(this->*functionPointer[i])(token, file);
				break;
			}
		}
		if (i == nbOfMethods)
			throw Location::Exception("Location construction error: somethind goes wrong");
	}
	// std::cout << "Name = |" << _name << "|\n";
	//	std::cout << "Root = |" << _root << "|\n";
	//	std::cout << "autoindex = " << _autoindex << std::endl;
	//	std::cout << "redirection [301] = " << _redirection[301] << std::endl;
	//	std::cout << "methods[0] = " << _methods[0] << std::endl;
	// if (!_index.empty())
	//	std::cout << "index[0] = " << _index[0] << std::endl;
}

Location::Location(const Location &former) { *this = former; }
Location &Location::operator=(const Location &former) {
	if (this != &former) {
		this->_name = former._name;
		this->_redirection = former._redirection;
		this->_methods = former._methods;
		this->_index = former._index;
		this->_root = former._root;
		this->_autoindex = former._autoindex;
		this->_clientMaxBodySize = former._clientMaxBodySize;
	}
	return *this;
}
Location::~Location() {}

Location::Exception::Exception(const std::string &message) : _errorMessage(message) {}

Location::Exception::~Exception() throw() {}

void Location::_parseRedirection(std::string &str, std::fstream &file) {
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
	if (!(issNum >> _redirection.second) || (issNum >> str))
		throw Location::Exception("Problem parse Redirection: invalid data");
}

void Location::_parseMethods(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception("Problem parse Methods");
	str = trim(str);
	if (str.length() - 1 != str.rfind(';'))
		throw Location::Exception("Problem parse Methods");
	str.erase(str.length() - 1);
	if (str.empty())
		throw Location::Exception("Problem parse Methods");

	std::istringstream iss(str);
	while (iss >> str) {
		_methods.push_back(str);
	}
}

void Location::_parseIndex(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception("Problem parse Index");
	str = trim(str);
	if (str.length() - 1 != str.rfind(';'))
		throw Location::Exception("Problem parse Index");
	str.erase(str.length() - 1);
	if (str.empty())
		throw Location::Exception("Problem parse Index");

	std::istringstream iss(str);
	while (iss >> str)
		_index.push_back(str);
}

void Location::_parseRoot(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception("Problem parse Root");
	_root = trim(str);
	if (_root.length() - 1 != _root.rfind(';'))
		throw Location::Exception("Problem parse Root");
	_root = _root.substr(0, _root.length() - 1);
	_root = trim(_root);
}

void Location::_parseIndent(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception("Problem parse Indent");
	str = trim(str);
	if (str.length() - 1 != str.rfind(';'))
		throw Location::Exception("Problem parse Indent");
	str = str.substr(0, str.length() - 1);
	str = trim(str);
	if (str == "on")
		_autoindex = true;
	else if (str == "off")
		_autoindex = false;
	else
		throw Location::Exception("Problem parse Indent: invalid data");
}

void Location::_parseClientMaxSizeBody(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception("Problem parse client max body size");
	str = trim(str);
	if (str.length() - 1 != str.find(';') || str.length() == 1)
		throw Location::Exception("Problem parse client max body size (;)");
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
		throw Location::Exception("client_max_body_size bad format.");
	if (multiplierLetter.empty())
		_clientMaxBodySize = val;
	else if (multiplierLetter == "k" || multiplierLetter == "K")
		_clientMaxBodySize = val * 1000;
	else if (multiplierLetter == "m" || multiplierLetter == "M")
		_clientMaxBodySize = val * 1000000;
	else if (multiplierLetter == "g" || multiplierLetter == "G")
		_clientMaxBodySize = val * 1000000000;
}

void Location::setDefaultMethods() {
	_methods.push_back("GET");
	_methods.push_back("DELETE");
	_methods.push_back("POST");
}

void Location::setDefaultIndex(const std::vector<std::string> &index) {
	if (!index.empty())
		_index = index;
}
void Location::setDefaultRoot(const std::string &root) { _root = root; }

const std::string                 &Location::getName() const { return _name; }
const std::pair<int, std::string> &Location::getRedirection() const { return _redirection; }
const std::vector<std::string>    &Location::getMethods() const { return _methods; };
const std::vector<std::string>    &Location::getIndex() const { return _index; };
const std::string                 &Location::getRoot() const { return _root; };
const bool                        &Location::getAutoindex() const { return _autoindex; };
const unsigned long &Location::getClientMaxSizeBody() const { return _clientMaxBodySize; };
