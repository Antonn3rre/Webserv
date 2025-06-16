#include "CgiHandler.hpp"
#include "Config.hpp"
#include "ResponseMessage.hpp"
#include "Server.hpp"
#include "cgiSession.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/epoll.h>
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
	envMap.insert(
	    std::pair<std::string, std::string>("HTTP_COOKIE", request.getHeaderValue("Cookie").first));
	// voir quoi rajouter d'autre

	std::vector<std::string> envVec;
	for (std::map<std::string, std::string>::iterator it = envMap.begin(); it != envMap.end();
	     ++it) {
		envVec.push_back(it->first + "=" + it->second);
	}
	return (envVec);
}

void CgiHandler::_extractHeader(ResponseMessage &response, const std::string &body,
                                const std::string &headerName) {
	size_t headerEnd = body.rfind("\r\n\r\n");
	size_t headerPos = body.find(headerName, 0);
	if (headerPos != std::string::npos && headerPos < headerEnd) {
		size_t valueStart = body.find_first_not_of(" \t", headerPos + headerName.size() + 1);
		size_t lineEnd = body.find("\r\n", valueStart);
		if (lineEnd != std::string::npos) {
			std::string headerValue = body.substr(valueStart, lineEnd - valueStart);
			response.addHeader(Header(headerName, headerValue));
		}
	}
}

void CgiHandler::divideCgiOutput(ResponseMessage &response) {
	std::string body = response.getBody();
	size_t      headerEnd = body.rfind("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return;

	_extractHeader(response, body, "Set-Cookie");
	_extractHeader(response, body, "Content-Type");
	body = body.substr(headerEnd + 4);
	response.setBody(body);
}

void CgiHandler::executeCgi(const std::string &uri, const Config &config, cgiSession &session,
                            Server &server, int eventFd) {
	struct stat sb;
	(void)config;
	if (access(uri.c_str(), F_OK) == -1)
		throw AMessage::MessageError(404);
	if (stat(uri.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR))
		throw AMessage::MessageError(404);
	// return (MethodHandler::getFileRequest(
	//     RequestHandler::findURILocation(config.getLocations(), request.getRequestUri()), uri));
	if (access(uri.c_str(), X_OK) == -1)
		throw AMessage::MessageError(403);

	// setEnv -> besoin de le faire ici car init en local
	std::vector<std::string> env = _setEnv(session.request, uri);
	std::vector<char *>      envp;
	envp.reserve(env.size());
	for (size_t i = 0; i < env.size(); ++i) {
		envp.push_back(const_cast<char *>(env[i].c_str()));
	}
	envp.push_back(NULL);

	int pipefdIn[2];
	int pipefdOut[2];
	if (pipe(pipefdIn) == -1 || pipe(pipefdOut) == -1) {
		std::cout << "erreur de pipe\n";
		throw std::exception();
	}

	session.setCgiPid(fork());
	if (session.getCgiPid() == 0) {
		dup2(pipefdIn[0], STDIN_FILENO);
		dup2(pipefdOut[1], STDOUT_FILENO);
		close(pipefdIn[1]);
		close(pipefdOut[0]);
		close(pipefdIn[0]);
		close(pipefdOut[1]);

		char *argv[] = {const_cast<char *>(uri.c_str()), NULL};
		execve(argv[0], argv, envp.data());

		std::cerr << "execve error" << std::endl;
		exit(EXIT_FAILURE);
	}
	close(pipefdIn[0]);
	close(pipefdOut[1]);
	//	std::cerr << "DEBUG (C++): Longueur du corps à écrire : " << request.getBody().length()
	//	          << std::endl;
	//	std::cerr << "DEBUG (C++): Contenu du corps à écrire : " << request.getBody() << std::endl;
	session.setPipeToCgi(pipefdIn[1]);
	session.setPipeFromCgi(pipefdOut[0]);

	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = session.getPipeFromCgi();
	epoll_ctl(server.getEpollFd(), EPOLL_CTL_ADD, session.getPipeFromCgi(), &ev);

	if (!session.request.getBody().empty()) {
		ev.events = EPOLLOUT | EPOLLET;
		ev.data.fd = session.getPipeToCgi();
		epoll_ctl(server.getEpollFd(), EPOLL_CTL_ADD, session.getPipeToCgi(), &ev);
	} else {
		close(session.getPipeToCgi());
		session.setPipeToCgi(-1);
	}

	server.cgiSessions[eventFd] = session;
	if (session.getPipeFromCgi() != -1) {
		server.cgiSessions[session.getPipeFromCgi()] = session;
	}

	// Mapper le FD du pipe d'écriture (vers le CGI)
	if (session.getPipeToCgi() != -1) {
		server.cgiSessions[session.getPipeToCgi()] = session;
	}
}
