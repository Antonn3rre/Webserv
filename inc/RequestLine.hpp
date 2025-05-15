#ifndef REQUESTLINE_HPP
#define REQUESTLINE_HPP

#include "AStartLine.hpp"
#include <string>

class RequestLine : public AStartLine {
	public:
	RequestLine(const std::string &line);
	RequestLine(const std::string &httpVersion, const std::string &method,
	            const std::string &requestUri);

	const std::string &getMethod() const;
	const std::string &getRequestUri() const;

	std::string str() const;

	private:
	std::string _method;
	std::string _requestUri;
};

#endif
