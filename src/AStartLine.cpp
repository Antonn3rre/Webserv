#include "AStartLine.hpp"

AStartLine::AStartLine(const std::string &version) : _httpVersion(version) {}

const std::string &AStartLine::getHttpVersion() const { return _httpVersion; }
