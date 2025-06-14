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
	static void executeCgi(const RequestMessage &request, const std::string &uri,
	                       const Config &config, Server &server, struct epoll_event &event);
	static void divideCgiOutput(ResponseMessage &response);
};
