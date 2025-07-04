#include "MethodHandler.hpp"
#include "AMessage.hpp"
#include "Config.hpp"
#include "RequestHandler.hpp"
#include "RequestMessage.hpp"
#include <algorithm>
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
		throw AMessage::MessageError(404);
	DIR *dir = opendir(page.c_str());
	if (!dir)
		throw AMessage::MessageError(403);
	const struct dirent *stDir;
	body += "<!DOCTYPE html>\
		<html lang=\"fr\"> \
		<body>";
	body += "<ul>";
	while (true) {
		stDir = readdir(dir);
		if (!stDir)
			break;
		body += "<li><a href=" + page.substr(7) + std::string(stDir->d_name) + ">" + stDir->d_name +
		        "</a></li>";
		body += "\n";
	}
	body += "</ul></body>";
	return (body);
}

std::string MethodHandler::getRequest(const RequestMessage &request, const std::string &page,
                                      const Config &config) {
	struct stat sb;
	if (access(page.c_str(), F_OK)) {
		throw AMessage::MessageError(404);
	}
	if (stat(page.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR))
		return (getFileRequest(
		    RequestHandler::findURILocation(config.getLocations(), request.getRequestUri()), page));
	if (access(page.c_str(), R_OK)) {
		throw AMessage::MessageError(403);
	}

	return (loadFile(page));
}

std::string MethodHandler::postRequest(const RequestMessage &request, const std::string &page) {
	if (!access(page.c_str(), F_OK) && access(page.c_str(), W_OK))
		throw AMessage::MessageError(403);
	if (request.getRequestUri() == "/upload") {
		_saveFile("test.txt", request.getBody());
	}
	return "";
}

std::string MethodHandler::deleteRequest(std::string &page) {
	std::string            body = loadFile(page);
	std::string            to_replace = "%20";
	std::string            replacement = " ";
	std::string::size_type pos = 0;
	while ((pos = page.find(to_replace, pos)) != std::string::npos) {
		page.replace(pos, to_replace.length(), replacement);
		pos += replacement.length();
	}

	if (access(page.c_str(), F_OK))
		throw AMessage::MessageError(404, "delete method", "file does not exist");
	if (access(page.c_str(), W_OK))
		throw AMessage::MessageError(403, "delete method", "cannot delete file");

	std::remove(page.c_str());

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
