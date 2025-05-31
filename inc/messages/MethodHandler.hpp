#pragma once

#include <string>
#include "RequestMessage.hpp"

class MethodHandler {
	private:
	MethodHandler();
	static void _executeMethod(const std::string &method);
	static void _saveFile(const std::string &filename, const std::string &body);

	public:
	static std::string deleteRequest(const std::string &page);
	static std::string getRequest(const std::string &page);
	static std::string postRequest(const RequestMessage &request, const std::string &page);

	static std::string loadFile(const std::string &filename);
};
