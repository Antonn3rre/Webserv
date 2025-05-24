#ifndef STATUSLINE_HPP
#define STATUSLINE_HPP

#include "AStartLine.hpp"
#include <map>
#include <string>

class StatusLine : public AStartLine {
	public:
	StatusLine(const std::string &line);
	StatusLine(const std::string &httpVersion, unsigned short statusCode);

	unsigned short     getStatusCode() const;
	const std::string &getReasonPhrase() const;

	std::string str() const;

	private:
	unsigned short                        _statusCode;
	std::string                           _reasonPhrase;
	std::map<unsigned short, std::string> _knownReasonPhrases;

	void               _setKnownReasonPhrases();
	static std::string _getReasonPhrase(unsigned short status);
};

#endif
