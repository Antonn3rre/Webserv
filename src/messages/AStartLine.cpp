#include "AStartLine.hpp"
#include "AMessage.hpp"

AStartLine::AStartLine(const std::string &version) : _httpVersion(version) {
	if (version.substr(0, 5) != "HTTP/")
		throw AMessage::MessageError(400);
	if (version != "HTTP/1.1")
		throw AMessage::MessageError(505);
}

AStartLine::~AStartLine() {}

const std::string &AStartLine::getHttpVersion() const { return _httpVersion; }
