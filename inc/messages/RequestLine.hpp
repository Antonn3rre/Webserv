#ifndef REQUESTLINE_HPP
#define REQUESTLINE_HPP

#include "AStartLine.hpp"
#include <map>
#include <string>

class RequestLine : public AStartLine {
	public:
	RequestLine();
	RequestLine(const std::string &line);
	RequestLine(const std::string &httpVersion, const std::string &method,
	            const std::string &requestUri);

	RequestLine &operator=(const RequestLine &other);

	const std::string                 &getMethod() const;
	const std::string                 &getRequestUri() const;
	const std::map<std::string, bool> &getValidMethods() const;

	void setUri(const std::string &uri);

	std::string str() const;

	private:
	std::string                 _method;
	std::string                 _requestUri;
	std::map<std::string, bool> _validMethods;

	void _setValidMethods();
};

#endif
