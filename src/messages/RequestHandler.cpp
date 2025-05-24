#include "RequestHandler.hpp"
#include "AMessage.hpp"
#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

RequestHandler::RequestHandler() {}

RequestHandler::RequestError::RequestError(const std::string &error, const std::string &argument)
    : AMessage::MessageError(error, argument) {}

unsigned short RequestHandler::RequestError::getStatusCode() const { return _statusCode; }

ResponseMessage RequestHandler::generateResponse(const Config         &config,
                                                 const RequestMessage &request) {
	unsigned short status;

	std::string     body = _generateBody(request, status);
	StatusLine      statusLine = _generateStatusLine(status);
	ResponseMessage response(statusLine, body);
	_generateHeaders(response, request);
	return response;
}

std::string RequestHandler::_generateBody(const RequestMessage &request, unsigned short &status) {}

StatusLine RequestHandler::_generateStatusLine(unsigned short status) {
	return StatusLine("HTTP/1.1", status);
}
void RequestHandler::_generateHeaders(ResponseMessage &response, const RequestMessage &request) {
	std::pair<std::string, bool> headerValue;
}

std::string RequestHandler::_executeCgi(const std::string &uri) {
	if (access(uri.c_str(), F_OK) == -1)
		throw AMessage::InvalidData("cgi, does not exist", uri);
	if (access(uri.c_str(), X_OK) == -1)
		throw AMessage::InvalidData("cgi, does not have authorization to execute", uri);
	int pipefd[2];
	pipe(pipefd);

	int pid = fork();
	if (pid == 0) {
		close(pipefd[0]);
		char **argv = {NULL};
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		execve(uri.c_str(), argv, NULL);
		std::cerr << "execve error" << std::endl;
		exit(EXIT_FAILURE);
	}
	close(pipefd[1]);
	ssize_t     bytesRead;
	std::string output;
	char        buffer[1024];
	bzero(buffer, 1024);
	do {
		bytesRead = read(pipefd[0], buffer, 1024);
		std::string bufStr(buffer);
		output += bufStr.substr(0, bytesRead);
	} while (bytesRead == 1024);
	close(pipefd[0]);
	return output;
}

std::string RequestHandler::_getCompletePath(const std::string &locRoot,
                                             const std::string &requestUri) {
	if (locRoot.empty())
		return (requestUri);
	if (*(locRoot.rbegin()) == '/')
		return (locRoot.substr(0, locRoot.size() - 1) + requestUri);
	return locRoot + requestUri;
}

std::string RequestHandler::_loadFile(const std::string &filename) {
	std::ifstream      file(filename.c_str(), std::ios::binary);
	std::ostringstream bodyStream;

	bodyStream << file.rdbuf();
	return bodyStream.str();
}

void RequestHandler::_saveFile(const std::string &filename, const std::string &body) {
	std::ofstream file;
	file.open(filename.c_str(), std::ios::trunc | std::ios::binary);
	file << body;
}

bool RequestHandler::_checkHost(const RequestMessage &request, const std::string &host) {
	std::pair<std::string, bool> hostValue = request.getHeaderValue("Host");

	if (!hostValue.second)
		return (false);
	return (hostValue.first == host);
}
