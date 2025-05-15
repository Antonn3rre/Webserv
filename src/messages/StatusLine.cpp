#include "StatusLine.hpp"
#include "AStartLine.hpp"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

StatusLine::StatusLine(const std::string &line) : AStartLine(line.substr(0, line.find(' '))) {
	size_t startPos = line.find(' ');
	size_t endPos = line.find(' ', startPos + 1);

	_statusCode = atoi(line.substr(startPos, endPos - startPos).c_str());
	_reasonPhrase = line.substr(endPos + 1, std::string::npos);
}

StatusLine::StatusLine(const std::string &httpVersion, unsigned short statusCode,
                       const std::string &reasonPhrase)
    : AStartLine(httpVersion), _statusCode(statusCode), _reasonPhrase(reasonPhrase) {}

unsigned short StatusLine::getStatusCode() const { return _statusCode; }

const std::string &StatusLine::getReasonPhrase() const { return _reasonPhrase; }

std::string StatusLine::str() const {
	std::stringstream sstream;

	sstream << getHttpVersion() << " " << _statusCode << " " << _reasonPhrase;
	return sstream.str();
}
