#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>

class Header {
	public:
	enum HeaderType {
		General,
		Request,
		Response,
		Entity,
	};

	Header(const std::string &headerLine);
	Header(const std::string &name, const std::string &value);
	Header(const std::string &name, HeaderType type, bool isSupported, bool isDuplicateAllowed);

	const HeaderType  &getType() const;
	const std::string &getName() const;
	const std::string &getValue() const;
	const bool        &isSupported() const;
	const bool        &isDuplicateAllowed() const;

	std::string str() const;

	private:
	std::string _name;
	std::string _value;
	HeaderType  _type;
	bool        _isSupported;
	bool        _isDuplicateAllowed;
};

#endif
