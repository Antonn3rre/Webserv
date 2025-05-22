#include "RequestHandler.hpp"
#include "AMessage.hpp"
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

	StatusLine statusLine = _generateStatusLine(status);
}

StatusLine RequestHandler::_generateStatusLine(unsigned short status) {
	return StatusLine("HTTP/1.1", status, _getReasonPhrase(status));
}

std::string RequestHandler::_getReasonPhrase(unsigned short status) {
	switch (status) {
		case 200:
			return "OK";
			break;
		case 201:
			return "Created";
			break;
		case 202:
			return "Accepted";
			break;
		case 203:
			return "Non-authoritative Information";
			break;
		case 204:
			return "No Content";
			break;
		case 205:
			return "Reset Content";
			break;
		case 206:
			return "Partial Content";
			break;
		case 300:
			return "Multiple Choices";
			break;
		case 301:
			return "Moved Permanently";
			break;
		case 302:
			return "Found";
			break;
		case 303:
			return "See Other";
			break;
		case 304:
			return "Not Modified";
			break;
		case 305:
			return "Use Proxy";
			break;
		case 306:
			return "Unused";
			break;
		case 307:
			return "Temporary Redirect";
			break;
		case 400:
			return "Bad Request";
			break;
		case 401:
			return "Unauthorized";
			break;
		case 402:
			return "Payment Required";
			break;
		case 403:
			return "Forbidden";
			break;
		case 404:
			return "Not Found";
			break;
		case 405:
			return "Method Not Allowed";
			break;
		case 406:
			return "Not Acceptable";
			break;
		case 407:
			return "Proxy Authentication Required";
			break;
		case 408:
			return "Request Timeout";
			break;
		case 409:
			return "Conflict";
			break;
		case 410:
			return "Gone";
			break;
		case 411:
			return "Length Required";
			break;
		case 412:
			return "Precondition failed";
			break;
		case 413:
			return "Request Entity Too Large";
			break;
		case 414:
			return "Request-url Too Long";
			break;
		case 415:
			return "Unsupported Media Type";
			break;
		case 416:
			return "Requested Rangee Not Satisfiable";
			break;
		case 417:
			return "Expectation Failed";
			break;
		case 500:
			return "Internal Server Error";
			break;
		case 501:
			return "Not Implemented";
			break;
		case 502:
			return "Bad Gateway";
			break;
		case 503:
			return "Service Unavailable";
			break;
		case 504:
			return "Gateway Timeout";
			break;
		case 505:
			return "HTTP Version Not Supported";
			break;
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

void saveFile(const std::string &filename, const std::string &body) {
	std::ofstream file;
	file.open(filename.c_str(), std::ios::trunc | std::ios::binary);
	file << body;
}
