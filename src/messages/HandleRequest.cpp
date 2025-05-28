#include "AMessage.hpp"
#include "Config.hpp"
#include "Location.hpp"
#include "RequestMessage.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <dirent.h>
#include <iterator>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

const Location &findURILocation(const std::deque<Location> &locations,
                                const RequestMessage       &request) {
	const std::string &uri = request.getRequestUri();
	const Location    *longestValidLoc = NULL;

	for (std::deque<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
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
	throw AMessage::MessageError(404);
}

bool checkMethods(const std::deque<std::string> &methods, const std::string &requestMethod) {
	for (std::deque<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
		if (*it == requestMethod)
			return (true);
	}
	return (false);
}
std::string getCompletePath(const std::string &locRoot, const std::string &requestUri) {
	if (locRoot.empty())
		return (requestUri);
	if (*(locRoot.rbegin()) == '/')
		return (locRoot.substr(0, locRoot.size() - 1) + requestUri);
	return locRoot + requestUri;
}

int checkUrl(std::string &url) {
	struct stat st;

	// check pour eviter les ../../psswd , voir si c'est ok
	if (url.find("..") != std::string::npos)
		return (403);
	if (url[0] == '/') {
		url = url.substr(1, url.size() - 1);
	}

	if (stat(url.c_str(), &st) == -1) {
		if (errno == EINVAL || errno == EACCES)
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

// recuperer index , return 0 si error, 1 si index, 2 si list a generer
int indexWork(const Config &config, std::string &url, Location &location) {
	std::string testIndex;

	// Si deque vide, begin == end
	std::deque<std::string> index = config.getIndex();
	for (std::deque<std::string>::iterator it = index.begin(); it != index.end(); ++it) {
		testIndex = url + *it;
		if (!access(testIndex.c_str(), F_OK)) {
			url = testIndex;
			return (1);
		}
	}
	if (location.getAutoindent())
		return (0);

	return (2); // liste autoindent a generer
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

// return std::pair<code, page> ?
std::pair<int, std::string> handleRequest(const Config &config, RequestMessage &request) {
	// check si host header est ok
	//	if (!checkHost(request.getHeaders(), server.getHost()))
	//		return std::make_pair(400, server.getErrorPage(400));

	// Trouver l'index de la bonne location
	Location location = findURILocation(config.getLocation(), request);

	// si un return -> renvoye direct le bon code erreur + page
	std::pair<int, std::string> returnInfo = location.getRedirection();
	if (returnInfo.first != -1)
		return (returnInfo); // type de retour a revoir

	// verif si method autorise
	if (!checkMethods(location.getMethods(), request.getMethod()))
		return std::make_pair(405, config.getErrorPage(405));

	// recuperer uri et construire chemin avec ro  (si aucun root defini ?)
	returnInfo.second = getCompletePath(location.getRoot(), request.getRequestUri());
	returnInfo.first = 200;

	// check si dossier, si oui envoyer sur index   // voir differents comportements selon
	// methode Si pas d'index, check autoindent et faire en fonction checkUrl -> return 0 si
	// dir, 1 si file, error code si error
	int resultCheckUrl = checkUrl(returnInfo.second);
	if (resultCheckUrl > 1)
		return std::make_pair(resultCheckUrl, config.getErrorPage(resultCheckUrl));

	// Si dossier -> envoye sur index ou autoindent
	if (!resultCheckUrl && request.getMethod() == "GET") {
		int resultIndex = indexWork(config, returnInfo.second, location);
		if (!resultIndex)
			return std::make_pair(403, config.getErrorPage(403));
		if (resultIndex == 2)
			returnInfo.first = 2; // si list a generer
	}
	// return si pas d'index et autoindent off

	// check si CGI dans ce cas la, pas de droits a verif je return direct
	// Pour l'instant on check pas, on executera avec python3, a voir ensuite
	if (returnInfo.second.find("cgi-bin") != std::string::npos)
		return std::make_pair(1, returnInfo.second);

	// Verifier si les droits sont les bons selon la methode
	int resultRights = checkRights(resultCheckUrl, returnInfo.second, request.getMethod());
	if (resultRights)
		return std::make_pair(resultRights, config.getErrorPage(resultRights));

	return (returnInfo);
}
