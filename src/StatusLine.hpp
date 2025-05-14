#ifndef STATUSLINE_HPP
#define STATUSLINE_HPP

#include "AStartLine.hpp"
#include <string>

class StatusLine : public AStartLine {
	private:
	unsigned short    _statusCode;
	const std::string _reasonPhrase;

	public:
	StatusLine(const std::string &method, const std::string &requestUri);

	unsigned short    &getStatusCode() const;
	const std::string &getReasonPhrase() const;

	std::string str() const;
};

#endif
