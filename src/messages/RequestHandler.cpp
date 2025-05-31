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
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

RequestHandler::RequestHandler() {}

RequestHandler::RequestError::RequestError(const std::string &error, const std::string &argument)
    : AMessage::MessageError(error, argument) {}

unsigned short RequestHandler::RequestError::getStatusCode() const { return _statusCode; }

ResponseMessage RequestHandler::generateResponse(const Config         &config,
                                                 const RequestMessage &request) {
	unsigned short status;
	(void)config;

	std::string     body = _generateBody(request, status);
	StatusLine      statusLine = _generateStatusLine(status);
	ResponseMessage response(statusLine, body);
	_generateHeaders(response, request);
	return response;
}

std::string RequestHandler::_generateBody(const RequestMessage &request, unsigned short &status) {
	(void)request;
	(void)status;
	return ("Fonction pas encore definie");
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

std::string RequestHandler::_deleteRequest(const std::string &page) {
	std::string body = _loadFile(page);
	std::remove(page.c_str());
	// add check ?
	return (body);
}

std::vector<std::string> RequestHandler::_setEnv(const RequestMessage &request,
                                             	const std::string	&uri) {
	std::map<std::string, std::string> envMap;
	envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", request.getMethod()));
	envMap.insert(std::pair<std::string, std::string>(
    	"CONTENT_TYPE", request.getHeaderValue("Content-Type").first));
	envMap.insert(std::pair<std::string, std::string>("SCRIPT_FILENAME", uri));
	envMap.insert(std::pair<std::string, std::string>("GATEWAY_INTERFACE", "CGI/1.1"));
	envMap.insert(std::pair<std::string, std::string>("REDIRECT_STATUS", "200"));
	envMap.insert(std::pair<std::string, std::string>("QUERY_STRING", ""));
	envMap.insert(std::pair<std::string, std::string>(
    	"CONTENT_LENGTH", request.getHeaderValue("Content-Length").first));
	envMap.insert(std::pair<std::string, std::string>("SERVER_NAME",
                                                  	request.getHeaderValue("server_name").first));
	// voir quoi rajouter d'autre

	std::vector<std::string> envVec;
	for (std::map<std::string, std::string>::iterator it = envMap.begin(); it != envMap.end();
     	++it) {
    	envVec.push_back(it->first + "=" + it->second);
	}
	return (envVec);
}

std::string RequestHandler::_executeCgi(const RequestMessage &request, const std::string &uri) {
	if (access(uri.c_str(), F_OK) == -1)
    	throw AMessage::InvalidData("cgi, does not exist", uri);
	if (access(uri.c_str(), X_OK) == -1)
    	throw AMessage::InvalidData("cgi, does not have authorization to execute", uri);

	// setEnv -> besoin de le faire ici car init en local
	std::vector<std::string> env = _setEnv(request, uri);
	std::vector<char *>  	envp;
	envp.reserve(env.size());
	for (size_t i = 0; i < env.size(); ++i) {
    	envp.push_back(const_cast<char *>(env[i].c_str()));
	}
	envp.push_back(NULL);

  int pipefdIn[2];
	int pipefdOut[2];
	pipe(pipefdIn);
	pipe(pipefdOut);

	int pid = fork();
	if (pid == 0) {
    	close(pipefdIn[1]);
    	close(pipefdOut[0]);
    	dup2(pipefdIn[0], STDIN_FILENO);
    	close(pipefdIn[0]);
    	dup2(pipefdOut[1], STDOUT_FILENO);
    	dup2(pipefdOut[1], STDERR_FILENO);
    	close(pipefdOut[1]);

    	char *argv[] = {const_cast<char *>("/usr/bin/php-cgi"), NULL};
    	execve("/usr/bin/php-cgi", argv, envp.data());
    	std::cerr << "execve error" << std::endl;
    	exit(EXIT_FAILURE);
	}
	close(pipefdIn[0]);
	close(pipefdOut[1]);
//	std::cerr << "DEBUG (C++): Longueur du corps à écrire : " << request.getBody().length()
//          	<< std::endl;
//	std::cerr << "DEBUG (C++): Contenu du corps à écrire :\n" << request.getBody() << std::endl;
	write(pipefdIn[1], request.getBody().c_str(), request.getBody().length());
	close(pipefdIn[1]);

	ssize_t 	bytesRead;
	std::string output;
	char    	buffer[1024];
	while ((bytesRead = read(pipefdOut[0], buffer, sizeof(buffer))) > 0)
    	output.append(buffer, bytesRead);
	close(pipefdOut[0]);

	int status;
	waitpid(pid, &status, 0);
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
