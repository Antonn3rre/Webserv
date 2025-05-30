#ifndef RESPONSEMESSAGE_HPP
#define RESPONSEMESSAGE_HPP

#include "AMessage.hpp"
#include "StatusLine.hpp"

class ResponseMessage : public AMessage {
	public:
	ResponseMessage(const std::string &message);
	ResponseMessage(const StatusLine &statusLine, const std::string &body);

	const std::string &getHttpVersion() const;
	unsigned short     getStatusCode() const;
	const std::string &getReasonPhrase() const;

	std::string str() const;

	private:
	const StatusLine _startLine;
};

#endif
