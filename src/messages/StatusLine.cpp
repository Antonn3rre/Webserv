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
    : AStartLine(httpVersion), _statusCode(statusCode) { 
  (void)reasonPhrase;
  _setKnownReasonPhrases();
  _reasonPhrase = getReasonPhrase(_statusCode);
}

void StatusLine::_setKnownReasonPhrases() {
	// 400-417    500-505   200
	_knownReasonPhrases.insert(std::make_pair(200, "OK"));
	_knownReasonPhrases.insert(std::make_pair(201, "Created"));
	_knownReasonPhrases.insert(std::make_pair(400, "Bad Request"));
	_knownReasonPhrases.insert(std::make_pair(401, "Unauthorized"));
	_knownReasonPhrases.insert(std::make_pair(402, "Payment Required"));
	_knownReasonPhrases.insert(std::make_pair(403, "Forbidden"));
	_knownReasonPhrases.insert(std::make_pair(404, "Not Found"));
	_knownReasonPhrases.insert(std::make_pair(405, "Method Not Allowed"));
	_knownReasonPhrases.insert(std::make_pair(406, "Not Acceptable"));
	_knownReasonPhrases.insert(std::make_pair(407, "Proxy Authentification Required"));
	_knownReasonPhrases.insert(std::make_pair(408, "Request Timeout"));
	_knownReasonPhrases.insert(std::make_pair(409, "Conflict"));
	_knownReasonPhrases.insert(std::make_pair(410, "Gone"));
	_knownReasonPhrases.insert(std::make_pair(411, "Length Required"));
	_knownReasonPhrases.insert(std::make_pair(412, "Precondition Failed"));
	_knownReasonPhrases.insert(std::make_pair(413, "Content Too Large"));
	_knownReasonPhrases.insert(std::make_pair(414, "URI Too Long"));
	_knownReasonPhrases.insert(std::make_pair(415, "Unsupported Media Type"));
	_knownReasonPhrases.insert(std::make_pair(416, "Range Not Satisfiable"));
	_knownReasonPhrases.insert(std::make_pair(417, "Expectation Failed"));
	_knownReasonPhrases.insert(std::make_pair(500, "Internal Servor Error"));
	_knownReasonPhrases.insert(std::make_pair(501, "Not Implemented"));
	_knownReasonPhrases.insert(std::make_pair(502, "Bad Gateway"));
	_knownReasonPhrases.insert(std::make_pair(503, "Service Unavailable"));
	_knownReasonPhrases.insert(std::make_pair(504, "Gateway Timeout"));
	_knownReasonPhrases.insert(std::make_pair(505, "HTTP Version Not Supported"));
}

unsigned short StatusLine::getStatusCode() const { return _statusCode; }

const std::string &StatusLine::getReasonPhrase(unsigned short code) const {
  std::cout << "ici\n";
	return _knownReasonPhrases.at(code);
}

const std::string &StatusLine::getReasonPhrase() const { return _reasonPhrase; }

std::string StatusLine::str() const {
	std::stringstream sstream;

	sstream << getHttpVersion() << " " << _statusCode << " " << _reasonPhrase;
	return sstream.str();
}
