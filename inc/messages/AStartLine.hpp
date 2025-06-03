#ifndef ASTARTLINE_HPP
#define ASTARTLINE_HPP

#include <string>

class AStartLine {
	public:
	AStartLine(const std::string &line);
	virtual ~AStartLine();

	AStartLine &operator=(const AStartLine &other);

	const std::string &getHttpVersion() const;

	virtual std::string str() const = 0;

	private:
	std::string _httpVersion;
};

#endif
