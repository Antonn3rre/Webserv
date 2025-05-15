#ifndef AMESSAGE_HPP
#define AMESSAGE_HPP

#include "Header.hpp"
#include <string>
#include <vector>

class AMessage {
	protected:
	std::vector<Header> _headers;
	std::string         _body;

	public:
	AMessage(const std::string &subMessage);
	AMessage(const std::string &body, const std::vector<Header> &headers);

	virtual const std::string &getHttpVersion() const = 0;
	virtual std::string        str() const = 0;
};

#endif
