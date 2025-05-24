#include "RequestHandler.hpp"
#include "AMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
RequestHandler::RequestHandler() {}

ResponseMessage RequestHandler::generateResponse(const Server         &server,
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
	return StatusLine("HTTP/1.1", status, _getReasonPhrase(status));
}
void RequestHandler::_generateHeaders(ResponseMessage &response, const RequestMessage &request) {
	std::pair<std::string, bool> headerValue;
}

std::string RequestHandler::_getReasonPhrase(unsigned short status) {
	switch (status) {
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 203:
			return "Non-authoritative Information";
		case 204:
			return "No Content";
		case 205:
			return "Reset Content";
		case 206:
			return "Partial Content";
		case 300:
			return "Multiple Choices";
		case 301:
			return "Moved Permanently";
		case 302:
			return "Found";
		case 303:
			return "See Other";
		case 304:
			return "Not Modified";
		case 305:
			return "Use Proxy";
		case 306:
			return "Unused";
		case 307:
			return "Temporary Redirect";
		case 400:
			return "Bad Request";
		case 401:
			return "Unauthorized";
		case 402:
			return "Payment Required";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 406:
			return "Not Acceptable";
		case 407:
			return "Proxy Authentication Required";
		case 408:
			return "Request Timeout";
		case 409:
			return "Conflict";
		case 410:
			return "Gone";
		case 411:
			return "Length Required";
		case 412:
			return "Precondition failed";
		case 413:
			return "Request Entity Too Large";
		case 414:
			return "Request-url Too Long";
		case 415:
			return "Unsupported Media Type";
		case 416:
			return "Requested Rangee Not Satisfiable";
		case 417:
			return "Expectation Failed";
		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 503:
			return "Service Unavailable";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
		default:
			throw std::invalid_argument("Invalid status in Request Handler");
	}
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
