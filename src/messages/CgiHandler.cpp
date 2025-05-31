#include "CgiHandler.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

CgiHandler::CgiHandler() {}

std::vector<char *> CgiHandler::_setEnv(const RequestMessage &request, const std::string &uri) {
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
std::string CgiHandler::executeCgi(const RequestMessage &request, const std::string &uri) {
	if (access(uri.c_str(), F_OK) == -1)
		throw AMessage::MessageError(404);
	if (access(uri.c_str(), X_OK) == -1)
		throw AMessage::MessageError(403);

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
