#include "AMessage.hpp"
#include "Header.hpp"
#include <cstddef>
#include <ios>
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
	// _checkDuplicateHeaders();
	// _checkHostHeader();

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

AMessage::MessageError::MessageError(unsigned short status) : _statusCode(status) {
	std::ostringstream msgStream;
	msgStream << "HTTP message error: " << status;
	_message = msgStream.str();
}

AMessage::MessageError::MessageError(unsigned short status, const std::string &error,
                                     const std::string &argument)
    : _statusCode(status) {
	std::ostringstream msgStream;
	msgStream << "HTTP message error " << status << ": " << error << ", " << argument;
	_message = msgStream.str();
}

AMessage::MessageError::~MessageError() throw() {}

const char *AMessage::MessageError::what() const throw() { return _message.c_str(); }

unsigned short AMessage::MessageError::getStatusCode() const { return _statusCode; }

AMessage &AMessage::operator=(const AMessage &other) {
	if (this != &other) {
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}

void AMessage::addHeader(const Header &header) {
	if (_isHeaderValid(header.getName()))
		_headers.push_back(header);
}

std::pair<std::string, bool> AMessage::getHeaderValue(const std::string &headerName) const {
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
		std::cerr << "Warning: unsupported header: " << headerName << std::endl;
		return false;
	}
	std::cerr << "Warning: invalid header: " << headerName << std::endl;
	return false;
}

void AMessage::_checkDuplicateHeaders() const {
	for (std::vector<Header>::const_iterator it1 = _headers.begin(); it1 != _headers.end(); ++it1)
		for (std::vector<Header>::const_iterator it2 = it1 + 1; it2 != _headers.end(); ++it2)
			if (it1->getName() == it2->getName() && !it1->isDuplicateAllowed())
				throw MessageError(400, "header (duplicate)", it1->str() + " and " + it2->str());
}

void AMessage::_checkHostHeader() const {
	std::vector<Header>::const_iterator it;
	for (it = _headers.begin(); it != _headers.end(); ++it) {
		if (it->getName() == "Host")
			break;
	}
	if (it == _headers.end())
		throw MessageError(400, "Host header", "missing");
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
	_insertKnownHeader("Transfer-Encoding", Header::General, true, true);
	_insertKnownHeader("Trailer", Header::General, true, false);
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
	_insertKnownHeader("Upgrade-Insecure-Requests", Header::Request, false, false);
	_insertKnownHeader("Sec-Fetch-Dest", Header::Request, false, false);
	_insertKnownHeader("Sec-Fetch-Mode", Header::Request, false, false);
	_insertKnownHeader("Sec-Fetch-Site", Header::Request, false, false);
	_insertKnownHeader("Sec-Fetch-User", Header::Request, false, false);
	_insertKnownHeader("Origin", Header::Request, false, false);
	_insertKnownHeader("Priority", Header::Request, false, false);
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

void AMessage::appendChunk(const std::string &chunk) {
	std::stringstream ss;
	size_t            sizeLen = chunk.find("\r\n");

	if (sizeLen == std::string::npos)
		throw AMessage::MessageError(400, "bad chunk", "missing CRLF after chunk size");
	ss << std::hex << chunk.substr(0, sizeLen);
	std::string s = ss.str();
	if (s.find_first_not_of("0123456789abcdefABCDEF") < sizeLen)
		throw AMessage::MessageError(400, "bad chunk", "chunk size is not an hexadecimal number");
	size_t chunkLen;
	ss >> chunkLen;
	if (ss.fail())
		throw AMessage::MessageError(400, "bad chunk", "chunk size is not an hexadecimal number");
	if (chunkLen != chunk.find("\r\n", sizeLen + 2) - sizeLen - 2)
		throw AMessage::MessageError(400, "bad chunk",
		                             "chunk size does not correspond to real size");
	std::string content = chunk.substr(sizeLen + 2, chunkLen);
	_body += content;
}

const std::string &AMessage::getBody() const { return _body; }

void AMessage::setBody(const std::string &str) { _body = str; }

const std::vector<Header> &AMessage::getHeaders() const { return _headers; }
