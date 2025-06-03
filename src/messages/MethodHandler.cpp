#include "MethodHandler.hpp"
#include "AMessage.hpp"
#include "Config.hpp"
#include "RequestHandler.hpp"
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

// getFileRequest   -> check autoindex + generer body
std::string MethodHandler::getFileRequest(const Location &loc, const std::string &page) {
	std::string body;

	if (!loc.getAutoindex())
		throw AMessage::MessageError(403);
	DIR *dir = opendir(page.c_str());
	if (!dir)
		throw AMessage::MessageError(403);
	const struct dirent *stDir;
	while (true) {
		stDir = readdir(dir);
		if (!stDir)
			break;
		body += stDir->d_name;
		body += "\n";
	}
	return (body);
}

std::string MethodHandler::getRequest(const std::string &page, const Config &config) {
	struct stat sb;
	if (access(page.c_str(), F_OK)) {
		throw AMessage::MessageError(404);
	}
	if (stat(page.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR))
		return (getFileRequest(RequestHandler::findURILocation(config.getLocations(), page), page));
	if (access(page.c_str(), R_OK)) {
		throw AMessage::MessageError(403);
	}

	return (loadFile(page));
}
/*
std::string MethodHandler::postRequest(const RequestMessage &request, const std::string &page) {
    if (!access(page.c_str(), F_OK) && access(page.c_str(), W_OK))
        throw AMessage::MessageError(403);
    std::string body = CgiHandler::executeCgi(request, page);
    return (body);
}
*/

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
