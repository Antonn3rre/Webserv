#include "RequestHandler.hpp"
#include "AMessage.hpp"
#include "Config.hpp"
#include "Header.hpp"
#include "Location.hpp"
#include "MethodHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
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

ResponseMessage RequestHandler::generateResponse(const Config         &config,
                                                 const RequestMessage &request) {
	unsigned short status;
	(void)config;

	std::cout << request.str() << std::endl;
	std::string     body = _generateBody(request, status, config);
	StatusLine      statusLine = _generateStatusLine(status);
	ResponseMessage response(statusLine, body);
	_generateHeaders(response, request, status);
	return response;
}

ResponseMessage RequestHandler::generateErrorResponse(const Config &config, unsigned short status) {
	std::string     body = _generateErrorBody(status, config);
	StatusLine      statusLine = _generateStatusLine(status);
	ResponseMessage response(statusLine, body);
	_generateErrorHeaders(response);
	return response;
}

std::string RequestHandler::_generateErrorBody(unsigned short status, const Config &config) {
	return MethodHandler::loadFile(config.getErrorPage(status));
}

void RequestHandler::_generateErrorHeaders(ResponseMessage &response) {
	_addDateHeader(response);
	response.addHeader(Header("Connection", "close"));
	response.addHeader(Header("Content-Type", "text/html"));
	_addContentLengthHeader(response);
}

std::string RequestHandler::_generateBody(const RequestMessage &request, unsigned short &status,
                                          const Config &config) {
	std::string        body;
	const std::string &method = request.getMethod();
	std::string        path = _getCompletePath(config, request.getRequestUri());

	try {
		if (method == "GET") {
			body = MethodHandler::getRequest(path);
		} else if (method == "POST") {
			body = MethodHandler::postRequest(request, path);
		} else if (method == "DELETE") {
			body = MethodHandler::deleteRequest(path);
		}
		status = 200;
	} catch (AMessage::MessageError &e) {
		status = e.getStatusCode();
		return _generateErrorBody(status, config);
	}
	return body;
}

StatusLine RequestHandler::_generateStatusLine(unsigned short status) {
	return StatusLine("HTTP/1.1", status);
}

void RequestHandler::_generateHeaders(ResponseMessage &response, const RequestMessage &request,
                                      unsigned short status) {
	// headerValue = _checkHost(request, "");
	_addDateHeader(response);
	_addContentLengthHeader(response);
	_addConnectionHeader(request, response);
	_addContentTypeHeader(request, response, status);
	response.addHeader(Header("Server", "webserv"));
}

void RequestHandler::_addDateHeader(ResponseMessage &response) {
	response.addHeader(Header("Date", _getTime()));
}

std::string RequestHandler::_getTime() {
	time_t   timeStamp = time(NULL);
	std::tm *dateTime = std::gmtime(&timeStamp);
	mktime(dateTime);
	std::string timeStr;

	// Week day
	switch (dateTime->tm_wday - 1) // -1 offset because wtf
	{
		case 0:
			timeStr += "Mon";
			break;
		case 1:
			timeStr += "Tue";
			break;
		case 2:
			timeStr += "Wed";
			break;
		case 3:
			timeStr += "Thu";
			break;
		case 4:
			timeStr += "Fri";
			break;
		case 5:
			timeStr += "Sat";
			break;
		case 6:
			timeStr += "Sun";
			break;
	}
	timeStr += ", ";

	std::ostringstream sstream;

	// Month day
	sstream << dateTime->tm_mday;
	timeStr += sstream.str() + " ";
	sstream.str("");

	// Month
	switch (dateTime->tm_mon) {
		case 0:
			timeStr += "Jan";
			break;
		case 1:
			timeStr += "Feb";
			break;
		case 2:
			timeStr += "Mar";
			break;
		case 3:
			timeStr += "Apr";
			break;
		case 4:
			timeStr += "May";
			break;
		case 5:
			timeStr += "Jun";
			break;
		case 6:
			timeStr += "Jul";
			break;
		case 7:
			timeStr += "Aug";
			break;
		case 8:
			timeStr += "Sep";
			break;
		case 9:
			timeStr += "Oct";
			break;
		case 10:
			timeStr += "Nov";
			break;
		case 11:
			timeStr += "Dec";
			break;
	}
	timeStr += " ";

	// Year
	sstream << dateTime->tm_year + 1900 << " ";
	// Hour
	sstream << dateTime->tm_hour << ":" << dateTime->tm_min << ":" << dateTime->tm_sec << " GMT";
	timeStr += sstream.str();

	return timeStr;
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

void RequestHandler::_addContentTypeHeader(const RequestMessage &request, ResponseMessage &response,
                                           unsigned short status) {
	if (status >= 400) {
		response.addHeader(Header("Content-Type", "text/html"));
		return;
	}
	const std::string &requestUri = request.getRequestUri();
	std::size_t        dotpos = requestUri.rfind('.', requestUri.length());
	if (dotpos != std::string::npos) {
		std::string extension = requestUri.substr(dotpos + 1, requestUri.length());
		if (extension == "JPG" || extension == "jpg")
			response.addHeader(Header("Content-Type", "image/jpeg"));
		else if (extension == "ico")
			response.addHeader(Header("Content-Type", "image/svg+xml"));
		else if (extension == "html")
			response.addHeader(Header("Content-Type", "text/html"));
	}
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
    	throw AMessage::MessageError("cgi, does not exist", uri);
	if (access(uri.c_str(), X_OK) == -1)
    	throw AMessage::MessageError("cgi, does not have authorization to execute", uri);

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

    if (uri.find(".php", uri.length() - 4)) {
    	char *argv[] = {const_cast<char *>("/usr/bin/php-cgi"), NULL};
    	execve("/usr/bin/php-cgi", argv, envp.data());
    }
    else if (uri.find(".py", uri.length() - 3)) {
    	char *argv[] = {const_cast<char *>("/usr/bin/python3"), NULL};
    	execve("/usr/bin/python3", argv, envp.data());
    }
    else if (uri.find(".cpp", uri.length() - 3)) {
    	char *argv[] = {const_cast<char *>("/usr/bin/cpp"), NULL};
    	execve("/usr/bin/cpp", argv, envp.data());
    }
    else
      std::cerr << "Format not supported" << std::endl;

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

std::string RequestHandler::_loadFile(const std::string &filename) {
	std::ifstream      file(filename.c_str(), std::ios::binary);
	std::ostringstream bodyStream;

	bodyStream << file.rdbuf();
	return bodyStream.str();
}

const Location &RequestHandler::_findURILocation(const std::vector<Location> &locations,
                                                 const std::string           &uri) {
	const Location *longestValidLoc = NULL;
  const Location *defaultLoc = NULL;

	for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end();
	     ++it) {
		if (it->getName().length() > uri.length())
			continue;
    if (it->getName() == "/")
      defaultLoc = &*it;
		std::string path = uri.substr(0, it->getName().length());
		if (*(path.end() - 1) != '/' && uri[path.length()] != '/')
			continue;
		if (it->getName() == path &&
		    (!longestValidLoc || it->getName().length() > longestValidLoc->getName().length()))
			longestValidLoc = &*it;
	}
	if (longestValidLoc) {
		return *longestValidLoc;
  }
  return *defaultLoc;
}

std::string RequestHandler::_getCompletePath(const Config &config, const std::string &requestUri) {
	std::string locRoot = _findURILocation(config.getLocations(), requestUri).getRoot();
	std::string path = locRoot + requestUri;
	struct stat sb;

	if (path[path.length() - 1] == '/')
		path += config.getIndex().at(0);
	else if (stat(path.c_str(), &sb) == 0 &&
	         (sb.st_mode & S_IFDIR)) // Is the path a directory but without the '/'
		path += "/" + config.getIndex().at(0);

	return path;
}
/*
std::string RequestHandler::_getCompletePath(const std::string &locRoot,
                                             const std::string &requestUri) {
	if (locRoot.empty())
		return (requestUri);
	if (*(locRoot.rbegin()) == '/')
		return (locRoot.substr(0, locRoot.size() - 1) + requestUri);
	return locRoot + requestUri;
}*/

bool RequestHandler::_checkHostHeader(const RequestMessage &request, const std::string &host) {
	std::pair<std::string, bool> hostValue = request.getHeaderValue("Host");

	if (!hostValue.second)
		return (false);
	return (hostValue.first == host);
}
