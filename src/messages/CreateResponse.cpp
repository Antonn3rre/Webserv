#include "Config.hpp"
#include "RequestHandler.hpp"
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

// Surement pas besoin de ces fonctions -> POST va dans CGI
// int checkContentType(std::string value) {
//	if (value.find("multipart/form-data;", 0, 20)) // pour upload fichier, ajouter autres si besoin
//		return (1);
//	return (415);
//}

// bool postMultipart(RequestMessage &request, const std::string &page, std::string &body) {
//	// add check boundary=
//	std::string contentType = request.getHeaderValue("Content-type").first;
//	std::string boundary =
//	    contentType.substr(contentType.find('=') + 1, contentType.size() - contentType.find('='));
//
//	// idee : faire une boucle pour recuperer les infos comme des RequestMessage
//	// (meme disposition de Header + body (parfois ?), pas de firstLine)
//	// Est ce qu'on stocke dans un RequestMessage ? Autre chose ?
//	// Est ce qu'on recupere tout puis traite tout, ou au fur et a mesure
//
//	(void)boundary;
//	(void)request;
//	(void)page;
//	(void)body;
//	return (true);
// }
//
// int postRequest(RequestMessage &request, const std::string &page, std::string &body) {
//	int contentType = checkContentType(request.getHeaderValue("Content-type").first);
//	if (contentType > 1)
//		return (contentType); // error
//	if (contentType == 1) {
//		postMultipart(request, page, body);
//	}
//	return (true);
// }

// Manque toute la construction des headers
ResponseMessage RequestHandler::createResponse(const Config &config, RequestMessage &request,
                                               std::pair<int, std::string> &handled) {
	std::string body;
	//	StatusLine  stLine(request.getHttpVersion(), handled.first, getReasonPhrase(handled.first));

	if (handled.first == 1) {
		body = _executeCgi(request, handled.second);
		handled.first = 200; // code a revoir
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
	} else if (request.getMethod() == "POST") {
		// return error en disant qu'on supporte pas ?
	}

	StatusLine      stLine(request.getHttpVersion(), handled.first);
	ResponseMessage response(stLine, body);
	return (response);
}
