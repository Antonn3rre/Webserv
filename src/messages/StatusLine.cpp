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

void StatusLine::_setKnownReasonPhrases() {
	// 400-417    500-505   200
	_reasonPhrase.insert(200, "OK");
	_reasonPhrase.insert(201, "Created");
	_reasonPhrase.insert(400, "Bad Request");
	_reasonPhrase.insert(401, "Unauthorized");
	_reasonPhrase.insert(402, "Payment Required");
	_reasonPhrase.insert(403, "Forbidden");
	_reasonPhrase.insert(404, "Not Found");
	_reasonPhrase.insert(405, "Method Not Allowed");
	_reasonPhrase.insert(406, "Not Acceptable");
	_reasonPhrase.insert(407, "Proxy Authentification Required");
	_reasonPhrase.insert(408, "Request Timeout");
	_reasonPhrase.insert(409, "Conflict");
	_reasonPhrase.insert(410, "Gone");
	_reasonPhrase.insert(411, "Length Required");
	_reasonPhrase.insert(412, "Precondition Failed");
	_reasonPhrase.insert(413, "Content Too Large");
	_reasonPhrase.insert(414, "URI Too Long");
	_reasonPhrase.insert(415, "Unsupported Media Type");
	_reasonPhrase.insert(416, "Range Not Satisfiable");
	_reasonPhrase.insert(417, "Expectation Failed");
	_reasonPhrase.insert(500, "Internal Server Error");
	_reasonPhrase.insert(501, "Not Implemented");
	_reasonPhrase.insert(502, "Bad Gateway");
	_reasonPhrase.insert(503, "Service Unavailable");
	_reasonPhrase.insert(504, "Gateway Timeout");
	_reasonPhrase.insert(505, "HTTP Version Not Supported");
}

unsigned short StatusLine::getStatusCode() const { return _statusCode; }

const std::string &StatusLine::getReasonPhrase(unsigned short code) const {
	return _knownReasonPhrases.at(code);
}

const std::string &StatusLine::getReasonPhrase() const { return _reasonPhrase; }

std::string StatusLine::str() const {
	std::stringstream sstream;

	sstream << getHttpVersion() << " " << _statusCode << " " << _reasonPhrase;
	return sstream.str();
}
