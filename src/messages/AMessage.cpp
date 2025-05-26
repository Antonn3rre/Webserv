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

	while (std::getline(sstream, line) && line != "\r") {
		addHeader(Header(line));
	}
	_checkDuplicateHeaders();
	_checkHostHeader();

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
	if (_isHeaderValid(header.getName()))
		_headers.push_back(header);
}

std::pair<std::string, bool> AMessage::getHeaderValue(const std::string &headerName) const {
	std::cout << "yessss\n";
	std::vector<Header>::const_iterator foundHeaderIterator = _headers.end();

	if (!_isHeaderValid(headerName))
		return (std::pair<std::string, bool>("", false));
	for (std::vector<Header>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		if (it->getName() == headerName) {
			if (foundHeaderIterator != _headers.end())
				return (std::pair<std::string, bool>("", false));
			foundHeaderIterator = it;
		}
	}
	if (foundHeaderIterator == _headers.end()) {
		return (std::pair<std::string, bool>("", false));
	}
	return (std::pair<std::string, bool>(foundHeaderIterator->getValue(), true));
}

bool AMessage::_isHeaderValid(const std::string &headerName) const {
	std::map<std::string, Header>::const_iterator headerEntry = _validHeaders.find(headerName);
	if (headerEntry != _validHeaders.end()) {
		if (headerEntry->second.isSupported())
			return true;
		// throw Unsupported("header", header.getName());
		std::cerr << "Warning: unsupported header: " << headerName << std::endl;
		return false;
	}
	std::cerr << "Warning: invalid header: " << headerName << std::endl;
	return false;
	//  else
	// 	throw InvalidData("header", header.getName());
}

void AMessage::_checkDuplicateHeaders() const {
	for (std::vector<Header>::const_iterator it1 = _headers.begin(); it1 != _headers.end(); ++it1)
		for (std::vector<Header>::const_iterator it2 = it1 + 1; it2 != _headers.end(); ++it2)
			if (it1->getName() == it2->getName() && !it1->isDuplicateAllowed())
				throw InvalidData("header (duplicate)", it1->str() + " and " + it2->str());
}

void AMessage::_checkHostHeader() const {
	std::vector<Header>::const_iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		if (it->getName() == "Host")
			break;
	}
	if (it == _headers.end())
		throw InvalidData("Host Header", "missing");
}

void AMessage::_insertKnownHeader(const std::string &name, Header::HeaderType type,
                                  bool isSupported, bool isDuplicateAllowed) {
	_validHeaders.insert(
	    std::pair<std::string, Header>(name, Header(name, type, isSupported, isDuplicateAllowed)));
}

void AMessage::_setValidHeaders() {
	// SUPPORTED
	// General headers
	_insertKnownHeader("Connection", Header::General, true, false);
	_insertKnownHeader("Date", Header::General, true, false);
	_insertKnownHeader("Transfer-Encoding", Header::General, true, false);
	// Entity headers
	_insertKnownHeader("Allow", Header::Entity, true, false);
	_insertKnownHeader("Content-Length", Header::Entity, true, false);
	_insertKnownHeader("Content-Type", Header::Entity, true, false);
	// Request headers
	_insertKnownHeader("Host", Header::Request, true, false);
	_insertKnownHeader("Cookie", Header::Request, true, false);
	// Response headers
	_insertKnownHeader("Server", Header::Response, true, false);
	_insertKnownHeader("Set-Cookie", Header::Response, true, true);

	// UNSUPPORTED
	// General headers
	_insertKnownHeader("Cache-Control", Header::General, false, true);
	_insertKnownHeader("Pragma", Header::General, false, false);
	_insertKnownHeader("Trailer", Header::General, false, false);
	_insertKnownHeader("Upgrade", Header::General, false, false);
	_insertKnownHeader("Via", Header::General, false, false);
	_insertKnownHeader("Warning", Header::General, false, false);
	// Entity headers
	_insertKnownHeader("Content-Encoding", Header::Entity, false, false);
	_insertKnownHeader("Content-Language", Header::Entity, false, false);
	_insertKnownHeader("Content-Location", Header::Entity, false, false);
	_insertKnownHeader("Content-MD5", Header::Entity, false, false);
	_insertKnownHeader("Content-Range", Header::Entity, false, false);
	_insertKnownHeader("Expires", Header::Entity, false, false);
	_insertKnownHeader("Last-Modified", Header::Entity, false, false);
	// Request headers
	_insertKnownHeader("Accept", Header::Request, false, false);
	_insertKnownHeader("Accept-Charset", Header::Request, false, false);
	_insertKnownHeader("Accept-Encoding", Header::Request, false, false);
	_insertKnownHeader("Accept-Language", Header::Request, false, false);
	_insertKnownHeader("Authorization", Header::Request, false, false);
	_insertKnownHeader("Expect", Header::Request, false, false);
	_insertKnownHeader("From", Header::Request, false, false);
	_insertKnownHeader("If-Match", Header::Request, false, false);
	_insertKnownHeader("If-Modified-Since", Header::Request, false, false);
	_insertKnownHeader("If-None-Match", Header::Request, false, false);
	_insertKnownHeader("If-Range", Header::Request, false, false);
	_insertKnownHeader("If-Unmodified-Since", Header::Request, false, false);
	_insertKnownHeader("Max-Forwards", Header::Request, false, false);
	_insertKnownHeader("Proxy-Authorization", Header::Request, false, false);
	_insertKnownHeader("Range", Header::Request, false, false);
	_insertKnownHeader("Referer", Header::Request, false, false);
	_insertKnownHeader("TE", Header::Request, false, false);
	_insertKnownHeader("User-Agent", Header::Request, false, false);
	// Response headers
	_insertKnownHeader("WWW-Authenticate", Header::Response, false, false);
	_insertKnownHeader("Access-Control-Allow-Origin", Header::Response, false, false);
	_insertKnownHeader("Access-Control-Allow-Credentials", Header::Response, false, false);
	_insertKnownHeader("Accept-Ranges", Header::Response, false, false);
	_insertKnownHeader("Age", Header::Response, false, false);
	_insertKnownHeader("ETag", Header::Response, false, false);
	_insertKnownHeader("Location", Header::Response, false, false);
	_insertKnownHeader("Proxy-Authenticate", Header::Response, false, false);
	_insertKnownHeader("Retry-After", Header::Response, false, false);
	_insertKnownHeader("Vary", Header::Response, false, false);
}

const std::string &AMessage::getBody() const { return _body; }

const std::vector<Header> &AMessage::getHeaders() const { return _headers; }
