#include "RequestMessage.hpp"
#include "AMessage.hpp"
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

RequestMessage::RequestMessage(const std::string &message)
    : AMessage((message.find("\r\n") != std::string::npos)
                   ? message.substr(message.find("\r\n") + 2,
                                    message.length() - message.find("\r\n") - 2)
                   : ""),
      _requestLine(message.substr(0, message.find("\r\n"))) {
	_readCookies();
}

RequestMessage::RequestMessage(const RequestLine &requestLine, const std::string &body)
    : AMessage(body, std::vector<Header>()), _requestLine(requestLine) {}

RequestMessage &RequestMessage::operator=(const RequestMessage &other) {
	if (this != &other) {
		AMessage::operator=(static_cast<const AMessage &>(other));
		_requestLine = other._requestLine;
	}
	return *this;
}

std::string RequestMessage::str() const {
	std::string str = _requestLine.str();
	str += "\r\n";
	for (std::vector<Header>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		str += (*it).str();
		str += "\r\n";
	}
	str += "\r\n";
	str += _body;

	return str;
}

void RequestMessage::_addCookie(const std::string &cookie) {
	size_t delimPos = cookie.find('=');
	if (delimPos == std::string::npos)
		return;
	_cookies.insert(std::pair<std::string, std::string>(
	    cookie.substr(0, delimPos), cookie.substr(delimPos + 1, cookie.length() - delimPos - 1)));
}

void RequestMessage::_readCookies() {
	for (std::vector<Header>::iterator it = _headers.begin(); it != _headers.end(); ++it) {
		if (it->getName() != "Cookie")
			continue;

		std::string delimiter("; ");
		long        start;
		long        end = -1 * static_cast<long>(delimiter.size());
		do {
			start = end + static_cast<long>(delimiter.size());
			end = static_cast<long>(it->getValue().find(delimiter, start));
			_addCookie(it->getValue().substr(start, end - start));
		} while (end != -1);
	}
}

void RequestMessage::displayCookies() {
	std::cout << "-- Request Cookies --" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = _cookies.begin();
	     it != _cookies.end(); ++it) {
		std::cout << it->first << "=" << it->second << std::endl;
	}
}

std::pair<std::string, bool> RequestMessage::getCookieValue(const std::string &name) const {
	std::pair<std::string, bool> result;
	result.first = "";
	result.second = false;
	std::map<std::string, std::string>::const_iterator valueIt = _cookies.find(name);
	if (valueIt == _cookies.end())
		return result;
	result.first = valueIt->second;
	result.second = true;
	return result;
}

const std::string &RequestMessage::getHttpVersion() const { return _requestLine.getHttpVersion(); }

const std::string &RequestMessage::getMethod() const { return _requestLine.getMethod(); }

const std::string &RequestMessage::getRequestUri() const { return _requestLine.getRequestUri(); }
