#pragma once

#include "RequestMessage.hpp"
#include <string>
#include <vector>

class CgiHandler {
	private:
	CgiHandler();

	static std::vector<char *> _setEnv(const RequestMessage &request, const std::string &uri);

	public:
	static std::string executeCgi(const RequestMessage &request, const std::string &uri);
};
