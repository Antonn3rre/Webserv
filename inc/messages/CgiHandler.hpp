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
	static std::string executeCgi(const RequestMessage &request, const std::string &uri,
	                              const Config &config);
	static void        divideCgiOutput(ResponseMessage &response);
};
