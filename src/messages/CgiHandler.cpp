#include "CgiHandler.hpp"
#include "Config.hpp"
#include "MethodHandler.hpp"
#include "RequestHandler.hpp"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

CgiHandler::CgiHandler() {}

std::vector<std::string> CgiHandler::_setEnv(const RequestMessage &request,
                                             const std::string    &uri) {
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
	// check si utile de recuperer server_name, si oui comment recuperer info
	envMap.insert(std::pair<std::string, std::string>("SERVER_NAME", "localhost"));
	// voir quoi rajouter d'autre

	std::vector<std::string> envVec;
	for (std::map<std::string, std::string>::iterator it = envMap.begin(); it != envMap.end();
	     ++it) {
		envVec.push_back(it->first + "=" + it->second);
	}
	return (envVec);
}

std::string CgiHandler::executeCgi(const RequestMessage &request, const std::string &uri,
                                   const Config &config) {
	struct stat sb;
	if (access(uri.c_str(), F_OK) == -1)
		throw AMessage::MessageError(404);
	if (stat(uri.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR))
		return (MethodHandler::getFileRequest(
		    RequestHandler::findURILocation(config.getLocations(), request.getRequestUri()), uri));
	if (access(uri.c_str(), X_OK) == -1)
		throw AMessage::MessageError(403);

	// setEnv -> besoin de le faire ici car init en local
	std::vector<std::string> env = _setEnv(request, uri);
	std::vector<char *>      envp;
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

		if (uri.find(".php", uri.length() - 4) != std::string::npos) {
			char *argv[] = {const_cast<char *>(uri.c_str()), NULL};
			execve(argv[0], argv, envp.data());
		} else if (uri.find(".py", uri.length() - 3) != std::string::npos) {
			char *argv[] = {const_cast<char *>(uri.c_str()), NULL};
			execve(argv[0], argv, envp.data());
		} else if (uri.find(".cgi", uri.length() - 4) != std::string::npos) {
			char *argv[] = {const_cast<char *>(uri.c_str()), NULL};
			execve(argv[0], argv, envp.data());
		} else
			std::cerr << "Format not supported" << std::endl;

		std::cerr << "execve error" << std::endl;
		exit(EXIT_FAILURE);
	}
	close(pipefdIn[0]);
	close(pipefdOut[1]);
	//	std::cerr << "DEBUG (C++): Longueur du corps à écrire : " << request.getBody().length()
	//	          << std::endl;
	//	std::cerr << "DEBUG (C++): Contenu du corps à écrire : " << request.getBody() << std::endl;
	write(pipefdIn[1], request.getBody().c_str(), request.getBody().length());
	close(pipefdIn[1]);

	ssize_t     bytesRead;
	std::string output;
	char        buffer[1024];
	while ((bytesRead = read(pipefdOut[0], buffer, sizeof(buffer))) > 0)
		output.append(buffer, bytesRead);
	close(pipefdOut[0]);

	int status;
	waitpid(pid, &status, 0);
	return output;
}
