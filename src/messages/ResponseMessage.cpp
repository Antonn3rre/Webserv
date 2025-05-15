#include "ResponseMessage.hpp"
#include "AMessage.hpp"
#include "StatusLine.hpp"
#include <vector>

ResponseMessage::ResponseMessage(const std::string &message)
    : AMessage(message.substr(message.find('\n') + 1, std::string::npos)),
      _startLine(message.substr(0, message.find('\n'))) {
	_setValidResponseHeaders();
}

ResponseMessage::ResponseMessage(const StatusLine &statusLine, const std::string &body)
    : AMessage(body, std::vector<Header>()), _startLine(statusLine) {
	_setValidResponseHeaders();
}

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

void ResponseMessage::_setValidResponseHeaders() {
	_validHeaders.push_back("Server");
	_validHeaders.push_back("Set-Cookie");
	_validHeaders.push_back("WWW-Authenticate");
}
