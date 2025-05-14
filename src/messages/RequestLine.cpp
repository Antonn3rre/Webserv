#include "RequestLine.hpp"
#include "AStartLine.hpp"
#include <cstddef>
#include <string>

RequestLine::RequestLine(const std::string &line)
    : AStartLine(line.substr(line.rfind(' ') + 1, std::string::npos)) {
	size_t spacePos = line.find(' ');

	_method = line.substr(0, spacePos);
	_requestUri = line.substr(spacePos + 1, std::string::npos);
}

const std::string &RequestLine::getMethod() const { return _method; }

const std::string &RequestLine::getRequestUri() const { return _requestUri; }

std::string RequestLine::str() const {
	return _method + " " + _requestUri + " " + getHttpVersion();
}
