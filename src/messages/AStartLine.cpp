#include "AStartLine.hpp"
#include "AMessage.hpp"
#include <cstddef>
#include <string>

AStartLine::AStartLine(const std::string &line) {
	size_t versionPos = line.find("HTTP");
	if (versionPos == std::string::npos)
		throw AMessage::MessageError(400, "invalid start line", "no http version");
	size_t      versionEnd = line.find_first_of(" \r\t", versionPos);
	std::string version = line.substr(versionPos, versionEnd - versionPos);
	if (version.length() < 5 || version.substr(0, 5) != "HTTP/")
		throw AMessage::MessageError(400, "invalid start line", "syntax error in http version");
	if (version != "HTTP/1.1")
		throw AMessage::MessageError(505, "unsupported", "http version");
	_httpVersion = version;
}

AStartLine::~AStartLine() {}

const std::string &AStartLine::getHttpVersion() const { return _httpVersion; }
