#include "AStartLine.hpp"

AStartLine::AStartLine(const std::string &version) : _httpVersion(version) {}

AStartLine::~AStartLine() {}

const std::string &AStartLine::getHttpVersion() const { return _httpVersion; }
