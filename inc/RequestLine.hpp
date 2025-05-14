#ifndef REQUESTLINE_HPP
#define REQUESTLINE_HPP

#include "AStartLine.hpp"
#include <string>

class RequestLine : public AStartLine {
	private:
	const std::string _method;
	const std::string _requestUri;

	public:
	RequestLine(const std::string &method, const std::string &requestUri);

	const std::string &getMethod() const;
	const std::string &getRequestUri() const;

	std::string str() const;
};

#endif
