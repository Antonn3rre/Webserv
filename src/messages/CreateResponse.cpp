#include "HandleRequest.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <fstream>
#include <string>
#include <unistd.h>

std::string readPage(std::string &page) {
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


ResponseMessage createResponse(Server &server, RequestMessage &request,
                               std::pair<int, std::string> &handled) {
	std::string body;
	//	StatusLine  stLine(request.getHttpVersion(), handled.first, getReasonPhrase(handled.first));
	StatusLine stLine(request.getHttpVersion(), handled.first);
	(void)server;

	if (handled.first == 1) {
		body = "CGI";
		// envoyer dans CGI
	} else if (handled.first != 200 || request.getMethod() == "GET") {
		// -1 a changer, la ca veut dire que c'est un code erreur
		body = readPage(handled.second);
	}
  else if (request.getMethod() == "DELETE") {
    body = deleteRequest(handled.second);
  }

	ResponseMessage response(stLine, body);
	return (response);
}
