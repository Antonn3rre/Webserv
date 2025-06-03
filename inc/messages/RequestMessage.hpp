#ifndef REQUESTMESSAGE_HPP
#define REQUESTMESSAGE_HPP

#include "AMessage.hpp"
#include "RequestLine.hpp"

class RequestMessage : public AMessage {
	public:
	RequestMessage(const std::string &message);
	RequestMessage(const RequestLine &requestLine, const std::string &body);

	RequestMessage &operator=(const RequestMessage &other);

	const std::string &getHttpVersion() const;
	const std::string &getMethod() const;
	const std::string &getRequestUri() const;

	std::string str() const;

	private:
	RequestLine _requestLine;
};

#endif
