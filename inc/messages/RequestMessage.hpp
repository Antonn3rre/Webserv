#ifndef REQUESTMESSAGE_HPP
#define REQUESTMESSAGE_HPP

#include "AMessage.hpp"
#include "RequestLine.hpp"

class RequestMessage : public AMessage {
	private:
	const RequestLine _startLine;

	public:
	RequestMessage(const std::string &message);

	const std::string &getHttpVersion() const;

	std::string str() const;
};

#endif
