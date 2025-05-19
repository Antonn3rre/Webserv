#include "AMessage.hpp"
#include "Header.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

AMessage::AMessage(const std::string &subMessage) {
	std::stringstream sstream;
	std::string       line;

	sstream << subMessage;

	_setValidHeaders();

	while (std::getline(sstream, line) && line != "") {
		addHeader(Header(line));
	}

	int i = 0;
	while (std::getline(sstream, line)) {
		if (i++ > 0)
			_body += "\n";
		_body += line;
	}
}

AMessage::AMessage(const std::string &body, const std::vector<Header> &headers) : _body(body) {
	_setValidHeaders();
	for (std::vector<Header>::const_iterator it(headers.begin()); it != _headers.end(); ++it)
		addHeader(*it);
}

AMessage::MessageError::MessageError(const std::string &error, const std::string &argument) {
	_message = "HTTP message error: " + error + ": " + argument;
}

AMessage::MessageError::~MessageError() throw() {}

const char *AMessage::MessageError::what() const throw() { return _message.c_str(); }

AMessage::SyntaxError::SyntaxError(const std::string &error, const std::string &badSyntax)
    : MessageError("bad syntax: " + error, badSyntax) {}

AMessage::Unsupported::Unsupported(const std::string &name, const std::string &value)
    : MessageError("unsupported " + name, value) {}

AMessage::InvalidData::InvalidData(const std::string &name, const std::string &value)
    : MessageError("invalid " + name, value) {}

void AMessage::addHeader(const Header &header) {
	std::map<std::string, std::pair<Header::HeaderType, bool> >::iterator headerEntry =
	    _validHeaders.find(header.getName());
	if (headerEntry != _validHeaders.end()) {
		if (headerEntry->second.second)
			_headers.push_back(header);
		else
			// throw Unsupported("header", header.getName());
			std::cerr << "Warning: unsupported header: " << header.getName() << std::endl;
	} else
		throw InvalidData("header", header.getName());
}

void AMessage::_insertKnownHeader(const std::string &name, Header::HeaderType type,
                                  bool isSupported) {
	_validHeaders.insert(std::pair<std::string, std::pair<Header::HeaderType, bool> >(
	    name, std::pair<Header::HeaderType, bool>(type, isSupported)));
}

void AMessage::_setValidHeaders() {
	// SUPPORTED
	// General headers
	_insertKnownHeader("Connection", Header::General, true);
	_insertKnownHeader("Date", Header::General, true);
	_insertKnownHeader("Transfer-Encoding", Header::General, true);
	// Entity headers
	_insertKnownHeader("Allow", Header::Entity, true);
	_insertKnownHeader("Content-Length", Header::Entity, true);
	_insertKnownHeader("Content-Type", Header::Entity, true);
	// Request headers
	_insertKnownHeader("Host", Header::Request, true);
	_insertKnownHeader("Cookie", Header::Request, true);
	// Response headers
	_insertKnownHeader("Server", Header::Response, true);
	_insertKnownHeader("Set-Cookie", Header::Response, true);

	// UNSUPPORTED
	// General headers
	_insertKnownHeader("Cache-Control", Header::General, false);
	_insertKnownHeader("Pragma", Header::General, false);
	_insertKnownHeader("Trailer", Header::General, false);
	_insertKnownHeader("Upgrade", Header::General, false);
	_insertKnownHeader("Via", Header::General, false);
	_insertKnownHeader("Warning", Header::General, false);
	// Entity headers
	_insertKnownHeader("Content-Encoding", Header::Entity, false);
	_insertKnownHeader("Content-Language", Header::Entity, false);
	_insertKnownHeader("Content-Location", Header::Entity, false);
	_insertKnownHeader("Content-MD5", Header::Entity, false);
	_insertKnownHeader("Content-Range", Header::Entity, false);
	_insertKnownHeader("Expires", Header::Entity, false);
	_insertKnownHeader("Last-Modified", Header::Entity, false);
	// Request headers
	_insertKnownHeader("Accept", Header::Request, false);
	_insertKnownHeader("Accept-Charset", Header::Request, false);
	_insertKnownHeader("Accept-Encoding", Header::Request, false);
	_insertKnownHeader("Accept-Language", Header::Request, false);
	_insertKnownHeader("Authorization", Header::Request, false);
	_insertKnownHeader("Expect", Header::Request, false);
	_insertKnownHeader("From", Header::Request, false);
	_insertKnownHeader("If-Match", Header::Request, false);
	_insertKnownHeader("If-Modified-Since", Header::Request, false);
	_insertKnownHeader("If-None-Match", Header::Request, false);
	_insertKnownHeader("If-Range", Header::Request, false);
	_insertKnownHeader("If-Unmodified-Since", Header::Request, false);
	_insertKnownHeader("Max-Forwards", Header::Request, false);
	_insertKnownHeader("Proxy-Authorization", Header::Request, false);
	_insertKnownHeader("Range", Header::Request, false);
	_insertKnownHeader("Referer", Header::Request, false);
	_insertKnownHeader("TE", Header::Request, false);
	_insertKnownHeader("User-Agent", Header::Request, false);
	// Response headers
	_insertKnownHeader("WWW-Authenticate", Header::Response, false);
	_insertKnownHeader("Access-Control-Allow-Origin", Header::Response, false);
	_insertKnownHeader("Access-Control-Allow-Credentials", Header::Response, false);
	_insertKnownHeader("Accept-Ranges", Header::Response, false);
	_insertKnownHeader("Age", Header::Response, false);
	_insertKnownHeader("ETag", Header::Response, false);
	_insertKnownHeader("Location", Header::Response, false);
	_insertKnownHeader("Proxy-Authenticate", Header::Response, false);
	_insertKnownHeader("Retry-After", Header::Response, false);
	_insertKnownHeader("Vary", Header::Response, false);
}
