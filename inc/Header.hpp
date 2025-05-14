#ifndef AHEADER_HPP
#define AHEADER_HPP

#include <string>

class Header {
	private:
	enum HeaderType {
		General,
		Request,
		Response,
		Entity,
	} const _type;
	const std::string _name;
	const std::string _value;

	public:
	Header(const std::string &headerLine);
	// AHeader(HeaderType type, const std::string &name, const std::string &value);

	std::string str() const;
};

#endif
