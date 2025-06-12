#include "Header.hpp"
#include "AMessage.hpp"
#include "utilsParsing.hpp"
#include <cstddef>

Header::Header() {}
Header::Header(const std::string &headerLine) {
	size_t nameLen = headerLine.find(':');
	if (nameLen == std::string::npos)
		throw AMessage::MessageError(400, "invalid header", "missing ':'");
	_name = (headerLine.substr(0, nameLen));
	size_t valueStart = headerLine.find_first_not_of(" \t", nameLen + 1);
	size_t valueEnd = headerLine.find_first_of('\r', valueStart);
	_value = (headerLine.substr(valueStart, valueEnd - valueStart));
	_value = trim(_value);
}

Header::Header(const std::string &name, const std::string &value) : _name(name), _value(value) {}

Header::Header(const std::string &name, HeaderType type, bool isSupported, bool isDuplicateAllowed)
    : _name(name), _type(type), _isSupported(isSupported), _isDuplicateAllowed(isDuplicateAllowed) {
}

Header &Header::operator=(const Header &other) {
	if (this != &other) {
		_type = other.getType();
		_name = other.getName();
		_value = other.getValue();
		_isSupported = other.isSupported();
		_isDuplicateAllowed = other.isDuplicateAllowed();
	}
	return (*this);
}

const Header::HeaderType &Header::getType() const { return _type; }
const std::string        &Header::getName() const { return _name; }
const std::string        &Header::getValue() const { return _value; }
const bool               &Header::isSupported() const { return _isSupported; }
const bool               &Header::isDuplicateAllowed() const { return _isDuplicateAllowed; }

std::string Header::str() const { return _name + ": " + _value; }
