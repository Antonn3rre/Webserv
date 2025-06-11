#pragma once

#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Server.hpp"
#include <string>
#include <vector>

class CgiHandler {
	private:
	CgiHandler();

	static std::vector<std::string> _setEnv(const RequestMessage &request, const std::string &uri);
	static void _extractHeader(ResponseMessage &response, const std::string &body,
	                           const std::string &headerName);

	public:
	// static std::string executeCgi(const RequestMessage &request, const std::string &uri,
	//                               const Config &config, int _epollfd);
	// static std::string executeCgi(const RequestMessage &request, const std::string &uri,
	//                               const Config &config, int _epollfd,
	//                               std::map<int, CgiContext> &cgiContexts);
	static void executeCgi(const RequestMessage &request, const std::string &uri,
	                       const Config &config, int _epollfd,
	                       std::map<int, CgiContext> &cgiContexts);
	static void divideCgiOutput(ResponseMessage &response);
};
