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
	AMessage(const std::string &message);

	const std::string &getHttpVersion() const;
	std::string        string() const;
};

#endif
