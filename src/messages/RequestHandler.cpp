#include "RequestHandler.hpp"
#include "AMessage.hpp"
#include "CgiHandler.hpp"
#include "Config.hpp"
#include "Header.hpp"
#include "Location.hpp"
#include "MethodHandler.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

RequestHandler::RequestHandler() {}

ResponseMessage RequestHandler::generateResponse(const Config         &config,
                                                 const RequestMessage &request, int clientFd) {
	unsigned short status;

	if (!_checkMethods(findURILocation(config.getLocations(), request.getRequestUri()).getMethods(),
	                   request.getMethod()))
		throw AMessage::MessageError(405, "Method not allowed in location", request.getMethod());
	std::string     body = _generateBody(request, status, config, clientFd);
	StatusLine      statusLine = generateStatusLine(status);
	ResponseMessage response(statusLine, body);
	generateHeaders(response, request, status);
	return response;
}

ResponseMessage RequestHandler::generateErrorResponse(const Config &config, unsigned short status) {
	std::string     body = _generateErrorBody(status, config);
	StatusLine      statusLine = generateStatusLine(status);
	ResponseMessage response(statusLine, body);
	_generateErrorHeaders(response);
	return response;
}

std::string RequestHandler::_generateErrorBody(unsigned short status, const Config &config) {
	return MethodHandler::loadFile(config.getErrorPage(status));
}

void RequestHandler::_generateErrorHeaders(ResponseMessage &response) {
	response.addDateHeader();
	response.addHeader(Header("Connection", "close"));
	response.addHeader(Header("Content-Type", "text/html"));
	response.addContentLengthHeader();
	response.addHeader(Header("Server", "webserv"));
}

std::string RequestHandler::_generateBody(const RequestMessage &request, unsigned short &status,
                                          const Config &config, int clientFd) {
	std::string body;

	if (request.getRequestUri()[0] != '/') {
		status = 400;
		return _generateErrorBody(status, config);
	}

	const std::string &method = request.getMethod();
	std::string        path = _getCompletePath(config, request.getRequestUri());

	try {
		status = 200;
		if (path.find("/cgi-bin/") != std::string::npos)
			throw CgiRequestException(request, clientFd, path, config);
		if (method == "DELETE")
			body = MethodHandler::deleteRequest(path);
		else if (method == "POST") {
			body = MethodHandler::postRequest(request, path);
		} else
			body = MethodHandler::getRequest(request, path, config);
	} catch (AMessage::MessageError &e) {
		status = e.getStatusCode();
		return _generateErrorBody(status, config);
	}
	return body;
}

StatusLine RequestHandler::generateStatusLine(unsigned short status) {
	return StatusLine("HTTP/1.1", status);
}

void RequestHandler::generateHeaders(ResponseMessage &response, const RequestMessage &request,
                                     unsigned short status) {
	response.addDateHeader();
	_addConnectionHeader(request, response);
	CgiHandler::divideCgiOutput(response);
	_addContentTypeHeader(request, response, status);
	response.addContentLengthHeader();
	response.addHeader(Header("Server", "webserv"));
	//	response.addSessionCookieHeader("test", "caca");
	//	response.addSessionCookieHeader("test2", "prout");
}

void RequestHandler::_addConnectionHeader(const RequestMessage &request,
                                          ResponseMessage      &response) {
	std::pair<std::string, bool> headerValue;

	headerValue = request.getHeaderValue("Connection");
	if (headerValue.first == "close")
		response.addHeader(Header("Connection", "close"));
	else
		response.addHeader(Header("Connection", "keep-alive"));
}

void RequestHandler::_addContentTypeHeader(const RequestMessage &request, ResponseMessage &response,
                                           unsigned short status) {
	// Si le content type a deja ete ajoute par le CGI
	//	CgiHandler::divideCgiOutput(response);

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

const Location &RequestHandler::findURILocation(const std::vector<Location> &locations,
                                                const std::string           &uri) {
	const Location *longestValidLoc = NULL;

	for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end();
	     ++it) {
		if (it->getName().length() > uri.length())
			continue;
		std::string path = uri.substr(0, it->getName().length());
		if (*(path.end() - 1) != '/' && (uri[path.length()] && uri[path.length()] != '/'))
			continue;
		if (it->getName() == path &&
		    (!longestValidLoc || it->getName().length() > longestValidLoc->getName().length()))
			longestValidLoc = &*it;
	}
	if (!longestValidLoc)
		throw Config::Exception("No default location found");
	return *longestValidLoc;
}

bool RequestHandler::_checkMethods(const std::vector<std::string> &methods,
                                   const std::string              &requestMethod) {
	return std::find(methods.begin(), methods.end(), requestMethod) != methods.end();
	// for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end();
	// ++it) 	if (*it == requestMethod) 		return (true); return (false);
}

std::string RequestHandler::_getCompletePath(const Config &config, const std::string &requestUri) {
	std::string locRoot = findURILocation(config.getLocations(), requestUri).getRoot();
	std::string path =
	    (locRoot[locRoot.size() - 1] == '/' ? locRoot.substr(0, locRoot.size() - 1) : locRoot) +
	    requestUri;
	struct stat sb;

	std::string testPath;
	if (stat(path.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR)) {
		int i = 0;
		if (path[path.length() - 1] != '/')
			path += '/';
		while (true) {
			try {
				testPath =
				    path + findURILocation(config.getLocations(), requestUri).getIndex().at(i);
			} catch (const std::out_of_range &e) {
				break;
			}
			if (!access(testPath.c_str(), F_OK) && !access(testPath.c_str(), R_OK))
				return (testPath);
			i++;
		}
	}
	return path;
}
