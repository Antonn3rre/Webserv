#include "AMessage.hpp"
#include "Header.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

AMessage::AMessage(const std::string &subMessage) {
	std::stringstream sstream;

	sstream << subMessage;

	std::string line;
	while (std::getline(sstream, line) && line != "") {
		_headers.push_back(Header(line));
	}

	while (std::getline(sstream, line)) {
		_body += line + "\n";
	}

	_setValidHeaders();
}

AMessage::AMessage(const std::string &body, const std::vector<Header> &headers) : _body(body) {
	_setValidHeaders();
	for (std::vector<Header>::const_iterator it(headers.begin()); it != _headers.end(); ++it) {
		addHeader(*it);
	}
}

void AMessage::addHeader(const Header &header) {
	for (std::vector<std::string>::iterator it(_validHeaders.begin()); it != _validHeaders.end();
	     ++it) {
		if (*it == header.getName()) {
			_headers.push_back(header);
			return;
		}
	}
	std::cout << "Invalid or unsupported header: " << header.getName() << std::endl;
}

void AMessage::_setValidHeaders() {
	// General Headers
	_validHeaders.push_back("Connection");
	_validHeaders.push_back("Date");
	_validHeaders.push_back("Transfer-Encoding");

	// Entity Headers
	_validHeaders.push_back("Allow");
	_validHeaders.push_back("Content-Length");
	_validHeaders.push_back("Content-Type");
}
