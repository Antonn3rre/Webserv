#ifndef HANDLE_REQUEST_HPP
#define HANDLE_REQUEST_HPP

#include "RequestMessage.hpp"
#include "Server.hpp"

int         findRightLocIndex(Server &server, RequestMessage &request);
int         checkMethods(std::deque<std::string> methods, const std::string &requestMethod);
std::string getCompletePath(const std::string &locRoot, const std::string &requestUri);
std::pair<int, std::string> handleRequest(Server &server, RequestMessage &request);
int                         checkUrl(const std::string &url);
int                         indexWork(Server &server, std::string &url, int indexLoc);
int checkRights(int type, const std::string &url, const std::string &method);

#endif // !HANDLE_REQUEST_HPP
