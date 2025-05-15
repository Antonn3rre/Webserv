#ifndef AMESSAGE_HPP
#define AMESSAGE_HPP

#include "Header.hpp"
#include <string>
#include <vector>

class AMessage {
	public:
	AMessage(const std::string &subMessage);
	AMessage(const std::string &body, const std::vector<Header> &headers);

	virtual const std::string &getHttpVersion() const = 0;

	void addHeader(const Header &header);

	virtual std::string str() const = 0;

	protected:
	std::vector<std::string> _validHeaders;
	std::vector<Header>      _headers;
	std::string              _body;

	private:
	void _setValidHeaders();
};

#endif
