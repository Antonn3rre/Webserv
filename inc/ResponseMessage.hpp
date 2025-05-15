#ifndef RESPONSEMESSAGE_HPP
#define RESPONSEMESSAGE_HPP

#include "AMessage.hpp"
#include "StatusLine.hpp"

class ResponseMessage : public AMessage {
	private:
	const StatusLine _startLine;

	public:
	ResponseMessage(const std::string &message);
	ResponseMessage(const StatusLine &statusLine, const std::string &body);

	const std::string &getHttpVersion() const;

	std::string str() const;
};

#endif
