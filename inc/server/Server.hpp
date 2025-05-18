#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config.hpp"
#include <netinet/in.h>
#include <sys/socket.h>

class Server {
	private:
	Config             _config;
	int                _msocket;
	struct sockaddr_in _server_addr;

	public:
	Server(void);
	Server(char *configFile);
	void startServer(void);

	// Config getter
	const std::string             &getListen() const;
	const std::deque<std::string> &getServerName() const;
	const std::string             &getErrorPage(int) const;
	const std::string             &getClientMaxBodySize() const;
	const std::string             &getHost() const;
	const std::string             &getRoot() const;
	const std::deque<std::string> &getIndex() const;
	const std::deque<Location>    &getLocation() const;

	// Location getter , int parameter is the index of the container
	const std::string             &getLocName(int) const;
	const std::string             &getLocRedirectionUri(int, int) const;
	const std::deque<std::string> &getLocMethods(int) const;
	const std::string             &getLocRoot(int) const;
	const bool                    &getLocAutoindent(int) const;

	// additionnal getter
	unsigned int getNumOfLoc() const;
};

#endif // SERVER_HPP
