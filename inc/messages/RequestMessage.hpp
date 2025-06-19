#ifndef REQUESTMESSAGE_HPP
#define REQUESTMESSAGE_HPP

#include "AMessage.hpp"
#include "RequestLine.hpp"
#include <map>
#include <string>
#include <utility>

class RequestMessage : public AMessage {
	public:
	RequestMessage();
	RequestMessage(const std::string &message);
	RequestMessage(const RequestLine &requestLine, const std::string &body);

	RequestMessage &operator=(const RequestMessage &other);

	const std::string           &getHttpVersion() const;
	const std::string           &getMethod() const;
	const std::string           &getRequestUri() const;
	std::pair<std::string, bool> getCookieValue(const std::string &name) const;

	void setUri(const std::string &uri);

	std::string str() const;

	void displayCookies();

	private:
	RequestLine                        _requestLine;
	std::map<std::string, std::string> _cookies;

	void _readCookies();
	void _addCookie(const std::string &cookie);
};

#endif
