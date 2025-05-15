#include "Header.hpp"

Header::Header(const std::string &headerLine)
    : _name(headerLine.substr(0, headerLine.find(':'))),
      _value(headerLine.substr(headerLine.find(' ') + 1, std::string::npos)) {}

Header::Header(const std::string &name, const std::string &value) : _name(name), _value(value) {}

Header::HeaderType Header::getType() const { return _type; }

std::string Header::str() const { return _name + ": " + _value; }
