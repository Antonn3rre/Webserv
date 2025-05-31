#include "Header.hpp"

Header::Header(const std::string &headerLine)
    : _name(headerLine.substr(0, headerLine.find(':'))),
      _value(headerLine.substr(headerLine.find(' ') + 1,
                               headerLine.find('\r') - headerLine.find(' ') - 1)) {}

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
