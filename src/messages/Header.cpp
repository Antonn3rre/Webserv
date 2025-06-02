#include "Header.hpp"
#include "AMessage.hpp"
#include <cstddef>

Header::Header(const std::string &headerLine) {
	size_t nameLen = headerLine.find(':');
	if (nameLen == std::string::npos)
		throw AMessage::MessageError(400, "invalid header", "missing ':'");
	_name = (headerLine.substr(0, nameLen));
	size_t valueStart = headerLine.find_first_not_of(" \t", nameLen + 1);
	size_t valueEnd = headerLine.find_first_of(" \t\r", valueStart);
	_value = (headerLine.substr(valueStart, valueEnd - valueStart));
}

Header::Header(const std::string &name, const std::string &value) : _name(name), _value(value) {}

Header::Header(const std::string &name, HeaderType type, bool isSupported, bool isDuplicateAllowed)
    : _name(name), _type(type), _isSupported(isSupported), _isDuplicateAllowed(isDuplicateAllowed) {
}

const Header::HeaderType &Header::getType() const { return _type; }
const std::string        &Header::getName() const { return _name; }
const std::string        &Header::getValue() const { return _value; }
const bool               &Header::isSupported() const { return _isSupported; }
const bool               &Header::isDuplicateAllowed() const { return _isDuplicateAllowed; }

std::string Header::str() const { return _name + ": " + _value; }
