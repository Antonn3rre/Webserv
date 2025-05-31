#include "MethodHandler.hpp"
#include "RequestHandler.hpp"
#include "AMessage.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

std::string MethodHandler::getRequest(const std::string &page) {
	if (access(page.c_str(), F_OK)) {
		throw AMessage::MessageError(404);
	}
	if (access(page.c_str(), R_OK)) {
		throw AMessage::MessageError(403);
	}

	return (loadFile(page));
}

std::string MethodHandler::postRequest(const RequestMessage &request, const std::string &page) {
	if (!access(page.c_str(), F_OK) && access(page.c_str(), W_OK))
		throw AMessage::MessageError(403);
	std::string body = RequestHandler::_executeCgi(request, page);
	return (body);
}

std::string MethodHandler::deleteRequest(const std::string &page) {
	if (access(page.c_str(), F_OK))
		throw AMessage::MessageError(404);
	if (access(page.c_str(), W_OK))
		throw AMessage::MessageError(403);

	std::string body = loadFile(page);
	std::remove(page.c_str());
	// add check ?
	return (body);
}

std::string MethodHandler::loadFile(const std::string &filename) {
	std::ifstream      file(filename.c_str(), std::ios::binary);
	std::ostringstream bodyStream;

	bodyStream << file.rdbuf();
	return bodyStream.str();
}

void MethodHandler::_saveFile(const std::string &filename, const std::string &body) {
	std::ofstream file;
	file.open(filename.c_str(), std::ios::trunc | std::ios::binary);
	file << body;
}
