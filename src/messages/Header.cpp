#include "Header.hpp"

Header::Header(const std::string &headerLine)
    : _name(headerLine.substr(0, headerLine.find(':'))),
      _value(headerLine.substr(headerLine.find(' ') + 1, headerLine.find('\r'))) {}

Header::Header(const std::string &name, const std::string &value) : _name(name), _value(value) {}

Header::HeaderType Header::getType() const { return _type; }
const std::string &Header::getName() const { return _name; }
const std::string &Header::getValue() const { return _value; }

std::string Header::str() const { return _name + ": " + _value; }
