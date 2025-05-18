// include si class
#include "Config.hpp"
#include "RequestMessage.hpp"
#include "Server.hpp"
#include <deque>
#include <string>

int findRightLocIndex(Server &server, RequestMessage &request) {
	std::pair<int, int> commonWord;
	commonWord.first = -1;
	int defaultLoc;

	for (int i = 0; i < (int)server.getNumOfLoc(); i++) {
		size_t locPos = 0;
		size_t uriPos = 0;
		int    word = 0;
		bool   mismatch = false;
		if (server.getLocName(i).size() > request.getRequestUri().size())
			continue;
		if (server.getLocName(i) == "/") {
			defaultLoc = i;
			continue;
		}
		if (server.getLocName(i).size() == request.getRequestUri().size() &&
		    server.getLocName(i) == request.getRequestUri())
			return (i);
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
			if (locNext == std::string::npos || uriNext == std::string::npos)
				break;
			locPos = locNext + 1;
			uriPos = uriNext + 1;
		}
		if (!mismatch && word > commonWord.second) {
			commonWord.first = i;
			commonWord.second = word;
		}
	}
	if (commonWord.first == -1)
		return defaultLoc;
	return commonWord.first;
}

int checkMethods(std::deque<std::string> methods, const std::string &requestMethod) {
	for (std::deque<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		if (*it == requestMethod)
			return (1);
	}
	return (0);
}

// return std::pair<code, page> ?
std::pair<int, std::string> handleRequest(Server &server, RequestMessage &request) {
	// iterer location de server pour attribuer le bon
	// stocker resultat temporaire dans std pair avec index et nmb de char en commun ?
	// check entre chaque /
	int indexLoc = findRightLocIndex(server, request);

	// si un return -> renvoye direct le bon code erreur + page
	std::pair<int, std::string> redir = server.getLocRedirection(indexLoc);
	if (redir.first != -1)
		return (redir); // type de retour a revoir

	// verif si method autorise
	if (!checkMethods(server.getLocMethods(indexLoc), request.getMethod()))
		throw Config::Exception("La methode n'est pas acceptee");
	// -> sinon return error
	// -> si methods existe pas ?? -> existera forcement avec les methodes par defaut (GET POST
	// DELETE)

	// recuperer uri et construire chemin avec root (si aucun root defini ?)

	// check si dossier, si oui envoyer sur index
	// Si pas d'index, check autoindent et faire en fonction

	// A PLACER : CGI
}
