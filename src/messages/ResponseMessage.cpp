#include "ResponseMessage.hpp"
#include "AMessage.hpp"

ResponseMessage::ResponseMessage(const std::string &message)
    : AMessage(message.substr(message.find('\n') + 1, std::string::npos)),
      _startLine(message.substr(0, message.find('\n'))) {}

const std::string &ResponseMessage::getHttpVersion() const { return _startLine.getHttpVersion(); }

std::string ResponseMessage::str() const {
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
