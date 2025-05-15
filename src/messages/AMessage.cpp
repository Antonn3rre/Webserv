#include "AMessage.hpp"
#include "Header.hpp"
#include <sstream>
#include <string>

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
}

AMessage::AMessage(const std::string &body, const std::vector<Header> &headers)
    : _headers(headers), _body(body) {}
