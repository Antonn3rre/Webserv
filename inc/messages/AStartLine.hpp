#ifndef ASTARTLINE_HPP
#define ASTARTLINE_HPP

#include <string>

class AStartLine {
	private:
	std::string _httpVersion;

	public:
	AStartLine(const std::string &line);
	virtual ~AStartLine();

	const std::string &getHttpVersion() const;

	virtual std::string str() const = 0;
};

#endif
