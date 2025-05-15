#include "RequestMessage.hpp"
#include "AMessage.hpp"
#include <vector>

RequestMessage::RequestMessage(const std::string &message)
    : AMessage(message.substr(message.find('\n') + 1, std::string::npos)),
      _startLine(message.substr(0, message.find('\n'))) {}

RequestMessage::RequestMessage(const RequestLine &requestLine, const std::string &body)
    : AMessage(body, std::vector<Header>()), _startLine(requestLine) {}

const std::string &RequestMessage::getHttpVersion() const { return _startLine.getHttpVersion(); }

std::string RequestMessage::str() const {
	std::string str = _startLine.str();
	str += "\n";
	for (std::vector<Header>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		str += (*it).str();
		str += "\n";
	}
	str += "\n";
	str += _body;

	return str;
}
