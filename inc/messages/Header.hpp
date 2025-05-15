#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>

class Header {
	public:
	Header(const std::string &headerLine);
	Header(const std::string &name, const std::string &value);

	enum HeaderType {
		General,
		Request,
		Response,
		Entity,
	};

	HeaderType         getType() const;
	const std::string &getName() const;
	const std::string &getValue() const;

	std::string str() const;

	private:
	const std::string _name;
	const std::string _value;
	HeaderType        _type;
};

#endif
