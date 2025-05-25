#include "HandleRequest.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

std::string readPage(const std::string &page) {
	std::fstream file;
	std::string  body;

	file.open(page.c_str(), std::fstream::in);
	if (!file.is_open()) {
		return ("Problem opening page"); // voir message erreur
	}
	getline(file, body, '\0');
	file.close();
	return (body);
}

std::string deleteRequest(std::string &page) {
	std::string body = readPage(page);
	std::remove(page.c_str());
	// add check ?
	return (body);
}

int generateAutoindent(std::string &page, std::string &body) {
	DIR           *dir = opendir(page.c_str());
	struct dirent *stDir;
	if (!dir)
		return (0);
	while (true) {
		stDir = readdir(dir);
		if (!stDir)
			break;
		body += stDir->d_name;
		body += "\n";
	}
	return (1);
}

ResponseMessage createResponse(const Config &config, RequestMessage &request,
                               std::pair<int, std::string> &handled) {
	std::string body;
	//	StatusLine  stLine(request.getHttpVersion(), handled.first, getReasonPhrase(handled.first));

	if (handled.first == 1) {
		body = "CGI";
		// envoyer dans CGI
	} else if (handled.first == 2) {
		handled.first = 200;
		if (!generateAutoindent(handled.second, body)) {
			body = readPage(config.getErrorPage(403));
			handled.first = 403;
		}
		// mauvais code mais ne devrait pas arriver jusque la
	} else if (handled.first != 200 || request.getMethod() == "GET") {
		// -1 a changer, la ca veut dire que c'est un code erreur
		body = readPage(handled.second);
	} else if (request.getMethod() == "DELETE") {
		body = deleteRequest(handled.second);
	}

	StatusLine      stLine(request.getHttpVersion(), handled.first);
	ResponseMessage response(stLine, body);
	return (response);
}
