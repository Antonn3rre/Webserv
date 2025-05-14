#ifndef ASTARTLINE_HPP
#define ASTARTLINE_HPP

#include <string>

class AStartLine {
	private:
	const std::string _httpVersion;

	public:
	AStartLine(const std::string &version);
	virtual ~AStartLine();

	const std::string &getHttpVersion() const;

	virtual std::string str() const = 0;
};

#endif
