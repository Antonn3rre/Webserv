#pragma once

#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Server.hpp"
#include "cgiSession.hpp"
#include <string>
#include <vector>

class CgiHandler {
	private:
	CgiHandler();

	static std::vector<std::string> _setEnv(const RequestMessage &request, const std::string &uri);
	static void _extractHeader(ResponseMessage &response, const std::string &body,
	                           const std::string &headerName);

	public:
	static void executeCgi(const std::string &uri, const Config &config, cgiSession &session,
	                       Server &server, int eventFd);
	static void divideCgiOutput(ResponseMessage &response);
};
