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
#include <dirent.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

RequestHandler::RequestHandler() {}

ResponseMessage RequestHandler::generateResponse(const Config         &config,
                                                 const RequestMessage &request) {
	unsigned short status;

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
			body = MethodHandler::postRequest(path);
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
	_addContentLengthHeader(response);
	_addConnectionHeader(request, response);
	_addContentTypeHeader(request, response, status);
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

const Location &RequestHandler::_findURILocation(const std::vector<Location> &locations,
                                                 const std::string           &uri) {
	const Location *longestValidLoc = NULL;

	for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end();
	     ++it) {
		if (it->getName().length() > uri.length())
			continue;
		std::string path = uri.substr(0, it->getName().length());
		if (*(path.end() - 1) != '/' && uri[path.length()] != '/')
			continue;
		if (it->getName() == path &&
		    (!longestValidLoc || it->getName().length() > longestValidLoc->getName().length()))
			longestValidLoc = &*it;
	}
	if (longestValidLoc)
		return *longestValidLoc;
	throw AMessage::MessageError(404, "requested URI does not correspond to any location", uri);
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

bool RequestHandler::_checkHostHeader(const RequestMessage &request, const std::string &host) {
	std::pair<std::string, bool> hostValue = request.getHeaderValue("Host");

	if (!hostValue.second)
		return (false);
	return (hostValue.first == host);
}
