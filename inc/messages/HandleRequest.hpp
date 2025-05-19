#ifndef HANDLE_REQUEST_HPP
#define HANDLE_REQUEST_HPP

#include "RequestMessage.hpp"
#include "Server.hpp"

int         findRightLocIndex(Server &server, RequestMessage &request);
int         checkMethods(std::deque<std::string> methods, const std::string &requestMethod);
std::string getCompletePath(const std::string &locRoot, const std::string &requestUri);
std::pair<int, std::string> handleRequest(Server &server, RequestMessage &request);

#endif // !HANDLE_REQUEST_HPP
