#pragma once

#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <string>
#include <vector>

class CgiHandler {
	private:
	CgiHandler();

	static std::vector<std::string> _setEnv(const RequestMessage &request, const std::string &uri);

	public:
	static int executeCgi(const RequestMessage &request, const Config &config, int epollfd);
	static ResponseMessage generateCgiResponse(const Config &config, const RequestMessage &request,
	                                           const std::string &output, int epollfd);
	static void            divideCgiOutput(ResponseMessage &response);
};
