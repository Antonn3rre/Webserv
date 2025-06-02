#include "RequestMessage.hpp"
#include "AMessage.hpp"
#include <vector>

RequestMessage::RequestMessage(const std::string &message)
    : AMessage(
          message.substr(message.find("\r\n") + 2, message.length() - message.find("\r\n") - 2)),
      _startLine(message.substr(0, message.find("\r\n"))) {}

RequestMessage::RequestMessage(const RequestLine &requestLine, const std::string &body)
    : AMessage(body, std::vector<Header>()), _startLine(requestLine) {}

const std::string &RequestMessage::getHttpVersion() const { return _startLine.getHttpVersion(); }

std::string RequestMessage::str() const {
	std::string str = _startLine.str();
	str += "\r\n";
	for (std::vector<Header>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		str += (*it).str();
		str += "\r\n";
	}
	str += "\r\n";
	str += _body;

	return str;
}

const std::string &RequestMessage::getMethod() const { return _startLine.getMethod(); }

const std::string &RequestMessage::getRequestUri() const { return _startLine.getRequestUri(); }
