// include si class
#include "RequestMessage.hpp"
#include "Server.hpp"
#include <string>

int findRightLocIndex(Server &server, RequestMessage &request) {
	int                 numOfLoc = (int)server.getNumOfLoc();
	std::pair<int, int> commonWord;
	commonWord.first = -1;

	size_t locPos = 0;
	size_t uriPos = 0;
	int    word = 0;
	bool   mismatch = false;
	int    defaulte;

	for (int i = 0; i < numOfLoc; i++) {
		if (server.getLocName(i).size() > request.getRequestUri().size())
			continue;
		if (server.getLocName(i) == "/") {
			defaulte = i;
			continue;
		}

		while (locPos < server.getLocName(i).size() && uriPos < request.getRequestUri().size()) {
			size_t      locNext = server.getLocName(i).find('/', locPos);
			std::string locSegment = server.getLocName(i).substr(locPos, locNext - locPos);

			size_t      uriNext = request.getRequestUri().find('/', uriPos);
			std::string uriSegment = request.getRequestUri().substr(uriPos, uriNext - uriPos);

			if (locSegment != uriSegment) {
				mismatch = true;
				break;
			}
			word++;

			locPos = locNext;
			uriPos = uriNext;
		}

		if (!mismatch && word > commonWord.first) {
			commonWord.first = i;
			commonWord.second = word;
		}
	}
	if (commonWord.first == -1)
		return defaulte;
	return commonWord.first;
}

// return std::pair<code, page> ?
void handleRequest(Server &server, RequestMessage &request) {
	// iterer location de server pour attribuer le bon
	// stocker resultat temporaire dans std pair avec index et nmb de char en commun ?
	// check entre chaque /
	int indexLoc = findRightLocIndex(server, request);
	(void)indexLoc;
	// Quand loc trouve, verifier si method autorise
	// -> sinon return error
	// -> si methods existe pas ??

	//!\ check si return ou method a la priorite
	// si un return -> renvoye direct le bon code erreur + page

	// recuperer uri et construire chemin avec root (si aucun root defini ?)

	// check si dossier, si oui envoyer sur index
	// Si pas d'index, check autoindent et faire en fonction

	// A PLACER : CGI
}

#include "RequestLine.hpp"
#include "RequestMessage.hpp"
#include "Server.hpp"
#include <iostream>

int findRightLocIndex(Server &server, RequestMessage &request) {
	int                 numOfLoc = (int)server.getNumOfLoc();
	std::pair<int, int> commonWord;
	commonWord.first = -1;

	size_t locPos = 0;
	size_t uriPos = 0;
	int    word = 0;
	bool   mismatch = false;
	int    defaulte;

	for (int i = 0; i < numOfLoc; i++) {
		std::cout << "server name= " << server.getLocName(i) << std::endl;
		locPos = 0;
		uriPos = 0;
		word = 0;
		mismatch = false;
		if (server.getLocName(i).size() > request.getRequestUri().size())
			continue;
		if (server.getLocName(i) == "/") {
			defaulte = i;
			continue;
		}
		if (server.getLocName(i).size() == request.getRequestUri().size() &&
		    server.getLocName(i) == request.getRequestUri())
			return (i);
		while (locPos < server.getLocName(i).size() && uriPos < request.getRequestUri().size()) {
			std::cout << "locPos = " << locPos << " uriPos = " << uriPos << std::endl;
			std::cout << "locSize = " << server.getLocName(i).size()
			          << " uriSize = " << request.getRequestUri().size() << std::endl;
			size_t      locNext = server.getLocName(i).find('/', locPos);
			std::string locSegment = server.getLocName(i).substr(locPos, locNext - locPos);

			size_t      uriNext = request.getRequestUri().find('/', uriPos);
			std::string uriSegment = request.getRequestUri().substr(uriPos, uriNext - uriPos);

			if (locSegment != uriSegment) {
				mismatch = true;
				break;
			}
			word++;

			if (locPos == std::string::npos)
				locPos = server.getLocName(i).size();
			else
				locPos = locNext + 1;
			if (uriPos == std::string::npos)
				uriPos = request.getRequestUri().size();
			else
				uriPos = uriNext + 1;
			/*
		if (locPos == std::string::npos || uriPos == std::string::npos)
			break;
		locPos = locNext + 1;
		uriPos = uriNext + 1;
	*/
		}
		std::cout << "mismatch = " << mismatch << "word = " << word << std::endl;
		if (!mismatch && word > commonWord.first) {
			commonWord.first = i;
			commonWord.second = word;
		}
	}
	if (commonWord.first == -1)
		return defaulte;
	return commonWord.first;
}

int main(int argc, char **argv) {
	if (argc != 2)
		return 0;
	try {
		Server         server(argv[1]);
		RequestLine    requestLine("GET /home/agozlan/test HTTP/1.1");
		RequestMessage requestMessage(requestLine, "");
		std::cout << "index = " << findRightLocIndex(server, requestMessage);
	} catch (Config::Exception &e) {
		std::cout << e.what() << std::endl;
	}
}
