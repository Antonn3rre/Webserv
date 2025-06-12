#include "RequestLine.hpp"
#include "AMessage.hpp"
#include "AStartLine.hpp"
#include <cstddef>
#include <map>
#include <string>

RequestLine::RequestLine() {}

RequestLine::RequestLine(const std::string &line) : AStartLine(line) {
	_setValidMethods();

	size_t spacePos = line.find(' ');
	if (spacePos == std::string::npos)
		throw AMessage::MessageError(400, "bad request line", "no space after method");
	_method = line.substr(0, spacePos);
	size_t spacePos2 = line.find(' ', spacePos + 1);
	if (spacePos2 == std::string::npos)
		throw AMessage::MessageError(400, "bad request line", "no space after uri");
	_requestUri = line.substr(spacePos + 1, spacePos2 - spacePos - 1);
	if (_requestUri.rfind("?") == _requestUri.length() - 1)
		_requestUri.resize(_requestUri.length() - 1);

	std::map<std::string, bool>::iterator headerEntry = _validMethods.find(_method);
	if (headerEntry == _validMethods.end())
		throw AMessage::MessageError(400, "invalid method", _method);
	if (!headerEntry->second)
		throw AMessage::MessageError(501, "unsupported method", _method);
}

RequestLine::RequestLine(const std::string &httpVersion, const std::string &method,
                         const std::string &requestUri)
    : AStartLine(httpVersion), _method(method), _requestUri(requestUri) {}

RequestLine &RequestLine::operator=(const RequestLine &other) {
	if (this != &other) {
		AStartLine::operator=(static_cast<const AStartLine &>(other));
		_method = other.getMethod();
		_requestUri = other.getRequestUri();
		_validMethods = other.getValidMethods();
	}
	return *this;
}

const std::string &RequestLine::getMethod() const { return _method; }

const std::string &RequestLine::getRequestUri() const { return _requestUri; }

std::string RequestLine::str() const {
	return _method + " " + _requestUri + " " + getHttpVersion();
}

const std::map<std::string, bool> &RequestLine::getValidMethods() const { return _validMethods; }

void RequestLine::_setValidMethods() {
	// SUPPORTED
	_validMethods.insert(std::pair<std::string, bool>("GET", true));
	_validMethods.insert(std::pair<std::string, bool>("POST", true));
	_validMethods.insert(std::pair<std::string, bool>("DELETE", true));

	// UNSUPPORTED
	_validMethods.insert(std::pair<std::string, bool>("HEAD", false));
	_validMethods.insert(std::pair<std::string, bool>("CONNECT", false));
	_validMethods.insert(std::pair<std::string, bool>("TRACE", false));
	_validMethods.insert(std::pair<std::string, bool>("OPTIONS", false));
}
