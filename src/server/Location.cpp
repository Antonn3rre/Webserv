#include "Location.hpp"
#include "utilsParsing.hpp"
#include "utilsSpace.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Location::Location(std::string &token, std::fstream &file) : _autoindent(false) {
	std::string list[] = {"return", "allow_methods", "root", "autoindent"};
	void (Location::*functionPointer[])(std::string &, std::fstream &) = {
	    &Location::_parseRedirection, &Location::_parseMethods, &Location::_parseRoot,
	    &Location::_parseIndent};

	getline(file, token);
	std::istringstream iss(token);
	if (!(iss >> token))
		throw Location::Exception();
	_name = token;
	if (!(iss >> token) || token != "{" || iss >> token)
		throw Location::Exception();
	while (true) {
		token = readToken(file);
		if (token == "}")
			break;
		if (token.empty())
			throw Location::Exception();
		for (int i = 0; i < 4; i++) {
			if (token == list[i]) {
				(this->*functionPointer[i])(token, file);
				break;
			}
			if (i == 4)
				throw Location::Exception();
		}
	}
	//	std::cout << "Name = |" << _name << "|\n";
	//	std::cout << "Root = |" << _root << "|\n";
	//	std::cout << "autoindent = " << _autoindent << std::endl;
	//	std::cout << "redirection [301] = " << _redirection[301] << std::endl;
	//	std::cout << "methods[0] = " << _methods[0] << std::endl;
}

Location::Location(const Location &former) { *this = former; }
Location &Location::operator=(const Location &former) {
	if (this != &former) {
		this->_name = former._name;
		this->_redirection = former._redirection;
		this->_methods = former._methods;
		this->_root = former._root;
		this->_autoindent = former._autoindent;
	}
	return *this;
}
Location::~Location() {}

const char *Location::Exception::what() const throw() { return ("Problem parsing location"); }

void Location::_parseRedirection(std::string &str, std::fstream &file) {
	std::string page;
	int         start;

	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception();
	str = trim(str);
	if (str.empty() || str[str.length() - 1] != ';')
		throw Location::Exception();
	str.erase(str.length() - 1);
	if (str.empty())
		throw Location::Exception();

	str = trim(str);
	start = (int)str.length() - 1;
	while (start >= 0 && !isSpace(str[start]))
		start--;
	if (start == -1)
		throw Location::Exception();
	page = str.substr(start + 1, str.length() - start);

	str = str.substr(0, start);
	std::istringstream iss(str);
	std::string        token;

	while (iss >> token) {
		std::istringstream issNum(token);
		int                code;
		if (!(issNum >> code) || code < 100 || code > 599)
			throw Location::Exception();
		_redirection[code] = page;
	}
}

void Location::_parseMethods(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception();
	str = trim(str);
	if (str.length() - 1 != str.rfind(';'))
		throw Location::Exception();
	str.erase(str.length() - 1);
	if (str.empty())
		throw Location::Exception();

	std::istringstream iss(str);
	while (iss >> str) {
		_methods.push_back(str);
	}
}
void Location::_parseRoot(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception();
	_root = trim(str);
	if (_root.length() - 1 != _root.rfind(';'))
		throw Location::Exception();
	_root = _root.substr(0, _root.length() - 1);
	_root = trim(_root);
}
void Location::_parseIndent(std::string &str, std::fstream &file) {
	std::getline(file, str);
	if (str.empty() || justSpaces(str))
		throw Location::Exception();
	str = trim(str);
	if (str.length() - 1 != str.rfind(';'))
		throw Location::Exception();
	str = str.substr(0, str.length() - 1);
	str = trim(str);
	if (str == "on")
		_autoindent = true;
	else if (str == "off")
		_autoindent = false;
	else
		throw Location::Exception();
}

std::string             Location::getName() const { return _name; }
std::string             Location::getRedirectionUri(int num) const { return _redirection.at(num); }
std::deque<std::string> Location::getMethods() const { return _methods; };
std::string             Location::getRoot() const { return _root; };
bool                    Location::getAutoindent() const { return _autoindent; };
