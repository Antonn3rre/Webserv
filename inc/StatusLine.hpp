#ifndef STATUSLINE_HPP
#define STATUSLINE_HPP

#include "AStartLine.hpp"
#include <string>

class StatusLine : public AStartLine {
	public:
	StatusLine(const std::string &line);
	StatusLine(const std::string &httpVersion, unsigned short &statusCode,
	           const std::string &reasonPhrase);

	unsigned short     getStatusCode() const;
	const std::string &getReasonPhrase() const;

	std::string str() const;

	private:
	unsigned short _statusCode;
	std::string    _reasonPhrase;
};

#endif
