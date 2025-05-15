#include "RequestLine.hpp"
#include "AStartLine.hpp"
#include <cstddef>
#include <string>

RequestLine::RequestLine(const std::string &line)
    : AStartLine(line.substr(line.rfind(' ') + 1, 8)) {
	size_t spacePos = line.find(' ');

	_method = line.substr(0, spacePos);
	_requestUri = line.substr(spacePos + 1, line.find(' ', spacePos + 1) - spacePos - 1);
}

RequestLine::RequestLine(const std::string &httpVersion, const std::string &method,
                         const std::string &requestUri)
    : AStartLine(httpVersion), _method(method), _requestUri(requestUri) {}

const std::string &RequestLine::getMethod() const { return _method; }

const std::string &RequestLine::getRequestUri() const { return _requestUri; }

std::string RequestLine::str() const {
	return _method + " " + _requestUri + " " + getHttpVersion();
}
