#include "RequestMessage.hpp"
#include "AMessage.hpp"
#include <vector>

RequestMessage::RequestMessage(const std::string &message)
    : AMessage((message.find("\r\n") != std::string::npos)
                   ? message.substr(message.find("\r\n") + 2,
                                    message.length() - message.find("\r\n") - 2)
                   : ""),
      _requestLine(message.substr(0, message.find("\r\n"))) {}

RequestMessage::RequestMessage(const RequestLine &requestLine, const std::string &body)
    : AMessage(body, std::vector<Header>()), _requestLine(requestLine) {}

RequestMessage &RequestMessage::operator=(const RequestMessage &other) {
	if (this != &other) {
		AMessage::operator=(static_cast<const AMessage &>(other));
		_requestLine = other._requestLine;
	}
	return *this;
}

std::string RequestMessage::str() const {
	std::string str = _requestLine.str();
	str += "\r\n";
	for (std::vector<Header>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		str += (*it).str();
		str += "\r\n";
	}
	str += "\r\n";
	str += _body;

	return str;
}

const std::string &RequestMessage::getHttpVersion() const { return _requestLine.getHttpVersion(); }

const std::string &RequestMessage::getMethod() const { return _requestLine.getMethod(); }

const std::string &RequestMessage::getRequestUri() const { return _requestLine.getRequestUri(); }
