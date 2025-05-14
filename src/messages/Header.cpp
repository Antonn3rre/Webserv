#include "Header.hpp"

Header::Header(const std::string &headerLine)
    : _name(headerLine.substr(0, headerLine.find(':'))),
      _value(headerLine.substr(headerLine.find(' '), std::string::npos)) {}

std::string Header::str() const { return _name + ": " + _value; }
