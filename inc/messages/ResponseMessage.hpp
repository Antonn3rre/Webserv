#ifndef RESPONSEMESSAGE_HPP
#define RESPONSEMESSAGE_HPP

#include "AMessage.hpp"
#include "StatusLine.hpp"

class ResponseMessage : public AMessage {
	public:
	ResponseMessage();
	ResponseMessage(const std::string &message);
	ResponseMessage(const StatusLine &statusLine, const std::string &body);

	ResponseMessage &operator=(const ResponseMessage &other);

	const std::string &getHttpVersion() const;
	unsigned short     getStatusCode() const;
	const std::string &getReasonPhrase() const;

	void addSessionCookieHeader(const std::string &name, const std::string &value);
	void addPermanentCookieHeader(const std::string &name, const std::string &value);
	void addDateHeader();
	void addContentLengthHeader();

	std::string str() const;

	private:
	StatusLine _startLine;

	static std::string _getTime();
};

#endif
