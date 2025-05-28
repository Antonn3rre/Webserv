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

	(void)config;
	std::cout << request.str() << std::endl;
	std::string     body = _generateBody(request, status);
	StatusLine      statusLine = _generateStatusLine(status);
	ResponseMessage response(statusLine, body);
	_generateHeaders(response, request);
	return response;
}

std::string RequestHandler::_generateBody(const RequestMessage &request, unsigned short &status) {
	std::string        body;
	const std::string &method = request.getMethod();
	if (method == "GET") {
		body = _getRequest(request.getRequestUri());
	} else if (method == "POST") {
		body = _postRequest(request.getRequestUri());
	} else if (method == "DELETE") {
		body = _deleteRequest(request.getRequestUri());
	}
	// (void)request;
	status = 200;
	return body;
}

StatusLine RequestHandler::_generateStatusLine(unsigned short status) {
	return StatusLine("HTTP/1.1", status);
}

void RequestHandler::_generateHeaders(ResponseMessage &response, const RequestMessage &request) {
	// headerValue = _checkHost(request, "");
	_addContentLengthHeader(response);
	_addConnectionHeader(request, response);
	response.addHeader(Header("Server", "webserv"));
}

void RequestHandler::_addConnectionHeader(const RequestMessage &request,
                                          ResponseMessage      &response) {
	std::pair<std::string, bool> headerValue;

	headerValue = request.getHeaderValue("Connection");
	if (headerValue.first == "close")
		// TODO: Close the client socket after sending request
		response.addHeader(Header("Connection", "close"));
	else
		response.addHeader(Header("Connection", "keep-alive"));
}

void RequestHandler::_addContentLengthHeader(ResponseMessage &response) {
	std::ostringstream lengthStream;
	lengthStream << response.getBody().length();
	response.addHeader(Header("Content-Length", lengthStream.str()));
}

std::string RequestHandler::_getRequest(const std::string &page) {
	std::string body = _loadFile(page);
	// add check ?
	return (body);
}

std::string RequestHandler::_postRequest(const std::string &page) {
	(void)page;
	std::string body;
	// CGI PHP anton
	return (body);
}

std::string RequestHandler::_deleteRequest(const std::string &page) {
	std::string body = _loadFile(page);
	std::remove(page.c_str());
	// add check ?
	return (body);
}

std::vector<char *> RequestHandler::_setEnv(const RequestMessage &request, const std::string &uri) {
	std::map<std::string, std::string> envMap;
	envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", request.getMethod()));
	envMap.insert(std::pair<std::string, std::string>(
	    "CONTENT_TYPE", request.getHeaderValue("Content-type").first));
	envMap.insert(std::pair<std::string, std::string>("SCRIPT_FILENAME", uri));
	envMap.insert(std::pair<std::string, std::string>("GATEWAY_INTERFACE", "CGI/1.1"));
	envMap.insert(std::pair<std::string, std::string>("REDIRECT_STATUS", "200"));
	envMap.insert(std::pair<std::string, std::string>(
	    "CONTENT_LENGTH", request.getHeaderValue("Content-Length").first));
	// voir quoi rajouter d'autre

	std::vector<std::string> envVec;
	std::vector<char *>      envp;
	for (std::map<std::string, std::string>::iterator it = envMap.begin(); it != envMap.end();
	     ++it) {
		envVec.push_back(it->first + "=" + it->second);
	}
	envp.reserve(envVec.size());
	for (size_t i = 0; i < envVec.size(); ++i) {
		envp.push_back(const_cast<char *>(envVec[i].c_str()));
	}
	envp.push_back(NULL);
	return (envp);
}

// manque
//  - Gestion des variables d'environnement (par la que passent les infos)
//  - waitpid ?
//  - Comment on gere si process bloquant ?
std::string RequestHandler::_executeCgi(const RequestMessage &request, const std::string &uri) {
	if (access(uri.c_str(), F_OK) == -1)
		throw AMessage::InvalidData("cgi, does not exist", uri);
	if (access(uri.c_str(), X_OK) == -1)
		throw AMessage::InvalidData("cgi, does not have authorization to execute", uri);

	std::vector<char *> envp = _setEnv(request, uri);
	// envoyer envp.data();

	int pipefd[2];
	pipe(pipefd);

	int pid = fork();
	if (pid == 0) {
		close(pipefd[0]);
		char *argv[] = {const_cast<char *>(uri.c_str()), NULL};
		// char *argv[] = {"php-cgi", NULL};
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		execve(uri.c_str(), argv, NULL);
		//	execve("/usr/bin/php-cgi", argv, envp);
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

bool RequestHandler::_checkHostHeader(const RequestMessage &request, const std::string &host) {
	std::pair<std::string, bool> hostValue = request.getHeaderValue("Host");

	if (!hostValue.second)
		return (false);
	return (hostValue.first == host);
}
