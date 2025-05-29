#ifndef HANDLE_REQUEST_HPP
#define HANDLE_REQUEST_HPP

#include "Application.hpp"
#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"

// class HandleRequest {
//	public:
int         findRightLocIndex(Config &config, RequestMessage &request);
int         checkMethods(std::vector<std::string> methods, const std::string &requestMethod);
std::string getCompletePath(const std::string &locRoot, const std::string &requestUri);
std::pair<int, std::string> handleRequest(const Config &config, RequestMessage &request);
int                         checkUrl(const std::string &url);
int                         indexWork(Config &config, std::string &url, int indexLoc);
int         checkRights(int type, const std::string &url, const std::string &method);
std::string executeCgi(const std::string &uri);

ResponseMessage createResponse(const Config &config, RequestMessage &request,
                               std::pair<int, std::string> &handled);

//};

#endif // !HANDLE_REQUEST_HPP
