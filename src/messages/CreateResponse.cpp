#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <dirent.h>
#include <fstream>
#include <string>
#include <unistd.h>

bool readPage(const std::string &page, std::string &body) {
	std::fstream file;

	file.open(page.c_str(), std::fstream::in);
	if (!file.is_open()) {
		return (false);
	}
	getline(file, body, '\0');
	file.close();
	return (true);
}

bool deleteRequest(const std::string &page, std::string &body) {
	if (!readPage(page, body))
		return (false);
	if (std::remove(page.c_str()))
		return (false);
	return (true);
}

bool generateAutoindent(const std::string &page, std::string &body) {
	DIR *dir = opendir(page.c_str());
	if (!dir)
		return (false);
	const struct dirent *stDir;
	while (true) {
		stDir = readdir(dir);
		if (!stDir)
			break;
		body += stDir->d_name;
		body += "\n";
	}
	return (true);
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
			readPage(config.getErrorPage(403), body); // 403 ou autre ?
			handled.first = 403;
		}
	} else if (handled.first != 200 || request.getMethod() == "GET") {
		if (!readPage(handled.second, body)) {
			readPage(config.getErrorPage(403), body); // mais si le read 403 fail aussi ??
			handled.first = 403;
		}
	} else if (request.getMethod() == "DELETE") {
		if (!deleteRequest(handled.second, body)) {
			readPage(config.getErrorPage(403), body); // mais si le read 403 fail aussi ??
			handled.first = 403;
		}
	}

	StatusLine      stLine(request.getHttpVersion(), handled.first);
	ResponseMessage response(stLine, body);
	return (response);
}
