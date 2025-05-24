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

StatusLine::StatusLine(const std::string &httpVersion, unsigned short statusCode)
    : AStartLine(httpVersion), _statusCode(statusCode) {
	_reasonPhrase = _getReasonPhrase(_statusCode);
}

std::string StatusLine::_getReasonPhrase(unsigned short status) {
	switch (status) {
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 203:
			return "Non-authoritative Information";
		case 204:
			return "No Content";
		case 205:
			return "Reset Content";
		case 206:
			return "Partial Content";
		case 300:
			return "Multiple Choices";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 303:
			return "See Other";
		case 304:
			return "Not Modified";
		case 305:
			return "Use Proxy";
		case 306:
			return "Unused";
		case 307:
			return "Temporary Redirect";
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorized";
		case 402:
			return "Payment Required";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 406:
			return "Not Acceptable";
		case 407:
			return "Proxy Authentication Required";
		case 408:
			return "Request Timeout";
		case 409:
			return "Conflict";
		case 410:
			return "Gone";
		case 411:
			return "Length Required";
		case 412:
			return "Precondition failed";
		case 413:
			return "Request Entity Too Large";
		case 414:
			return "Request-url Too Long";
		case 415:
			return "Unsupported Media Type";
		case 416:
			return "Requested Rangee Not Satisfiable";
		case 417:
			return "Expectation Failed";
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
		default:
			throw std::invalid_argument("Invalid status in Request Handler");
	}
}

unsigned short StatusLine::getStatusCode() const { return _statusCode; }

const std::string &StatusLine::getReasonPhrase() const { return _reasonPhrase; }

std::string StatusLine::str() const {
	std::stringstream sstream;

	sstream << getHttpVersion() << " " << _statusCode << " " << _reasonPhrase;
	return sstream.str();
}
