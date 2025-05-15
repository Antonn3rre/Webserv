#ifndef AMESSAGE_HPP
#define AMESSAGE_HPP

#include "Header.hpp"
#include <map>
#include <string>
#include <utility>
#include <vector>

class AMessage {
	public:
	AMessage(const std::string &subMessage);
	AMessage(const std::string &body, const std::vector<Header> &headers);

	virtual const std::string &getHttpVersion() const = 0;

	void addHeader(const Header &header);

	virtual std::string str() const = 0;

	protected:
	std::map<std::string, std::pair<Header::HeaderType, bool> > _knownHeaders;
	std::vector<Header>                                         _headers;
	std::string                                                 _body;

	private:
	void _insertKnownHeader(const std::string &name, Header::HeaderType type, bool isSupported);
	void _setValidHeaders();
};

#endif
