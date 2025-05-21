// include si class
#include "AMessage.hpp"
#include "RequestMessage.hpp"
#include "Server.hpp"
#include <cerrno>
#include <deque>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

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

int checkMethods(const std::deque<std::string> &methods, const std::string &requestMethod) {
	for (std::deque<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
		if (*it == requestMethod)
			return (1);
	}
	return (0);
}
std::string getCompletePath(const std::string &locRoot, const std::string &requestUri) {
	if (locRoot.empty())
		return (requestUri);
	return locRoot + requestUri;
}

int checkUrl(std::string &url) {
	struct stat st;

	if (url[0] == '/') {
		url = url.substr(1, url.size() - 1);
	}

	if (stat(url.c_str(), &st) == -1) {
		if (errno == EINVAL | errno == EACCES)
			return (403);
		if (errno == ELOOP)
			return (508);
		if (errno == ENOENT || errno == ENOTDIR)
			return (404);
		return (500);
	}
	if (S_ISDIR(st.st_mode))
		return (0); // c'est un dir
	if (S_ISREG(st.st_mode))
		return (1); // c'est un file
	return (403);
}

// recuperer index
int indexWork(Server &server, std::string &url, int indexLoc) {
	std::string testIndex;

	// Si deque vide, begin == end
	std::deque<std::string> index = server.getIndex();
	for (std::deque<std::string>::iterator it = index.begin(); it != index.end(); it++) {
		testIndex = url + *it;
		if (!access(testIndex.c_str(), F_OK)) {
			url = testIndex;
			return (1);
		}
	}
	if (!server.getLocAutoindent(indexLoc))
		return (0);

	// ajouter liste autoindent
	return (0); // a changer en 1 quand fonction finie
}

int checkRights(int type, const std::string &url, const std::string &method) {
	// Si GET -> check si lecture possible
	// Si POST ou DELETE-> check si modification possible

	if (method == "GET") {
		if (access(url.c_str(), R_OK))
			return (403);
		return (0);
	}
	if (type == 1) { // file
		if (access(url.c_str(), W_OK))
			return (403);
	} else { // dir
		struct stat st;
		stat(url.c_str(), &st);
		if (!(st.st_mode & S_IWUSR))
			return (403);
	}
	return (0);
}

std::string loadFile(const std::string &filename) {
	std::ifstream     file(filename.c_str());
	std::string       line;
	std::stringstream bodyStream;

	while (std::getline(file, line))
		bodyStream << line;
	return bodyStream.str();
}

void saveFile(const std::string &filename, const std::string &body) {
	std::ofstream file;
	file.open(filename.c_str(), std::ios::trunc);

	std::stringstream bodyStream(body);
	std::string       line;

	while (std::getline(bodyStream, line))
		file << line;
}

void executeCgi(const std::string &uri) {
	if (access(uri.c_str(), F_OK) == -1)
		throw AMessage::InvalidData("cgi, does not exist", uri);
	if (access(uri.c_str(), X_OK) == -1)
		throw AMessage::InvalidData("cgi, does not have authorization to execute", uri);
	int pid = fork();
	if (pid == 0) {
		char **argv = {NULL};
		execve(uri.c_str(), argv, NULL);
	}
}

// return std::pair<code, page> ?
std::pair<int, std::string> handleRequest(Server &server, RequestMessage &request) {
	// iterer location de server pour attribuer le bon ??

	// Trouver l'index de la bonne location
	int indexLoc = findRightLocIndex(server, request);

	// si un return -> renvoye direct le bon code erreur + page
	std::pair<int, std::string> returnInfo = server.getLocRedirection(indexLoc);
	if (returnInfo.first != -1)
		return (returnInfo); // type de retour a revoir

	// verif si method autorise
	if (!checkMethods(server.getLocMethods(indexLoc), request.getMethod()))
		return std::make_pair(405, server.getErrorPage(405));

	// recuperer uri et construire chemin avec ro  (si aucun root defini ?)
	returnInfo.second = getCompletePath(server.getLocRoot(indexLoc), request.getRequestUri());

	// check si dossier, si oui envoyer sur index   // voir differents comportements selon
	// methode Si pas d'index, check autoindent et faire en fonction checkUrl -> return 0 si
	// dir, 1 si file, error code si error
	// /!\ pas securise (on peut chercher ../../../../../etc/passwd)
	int resultCheckUrl = checkUrl(returnInfo.second);
	if (resultCheckUrl > 1)
		return std::make_pair(resultCheckUrl, server.getErrorPage(resultCheckUrl));

	// Si dossier -> envoye sur index ou autoindent   // /!\ a faire seulement si GET
	if (!resultCheckUrl && request.getMethod() == "GET" &&
	    !indexWork(server, returnInfo.second, indexLoc))
		return std::make_pair(403, server.getErrorPage(403));
	// return si pas d'index et autoindent off

	// check si CGI dans ce cas la, pas de droits a verif je return direct
	// Pour l'instant on check pas, on executera avec python3, a voir ensuite
	if (server.getLocName(indexLoc).find("cgi-bin") != std::string::npos)
		return std::make_pair(1, returnInfo.second);

	// Verifier si les droits sont les bons selon la methode
	int resultRights = checkRights(resultCheckUrl, returnInfo.second, request.getMethod());
	if (resultRights)
		return std::make_pair(resultRights, server.getErrorPage(resultRights));

	return (returnInfo);
}
