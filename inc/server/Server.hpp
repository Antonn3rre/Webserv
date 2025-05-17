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
	std::string             getListen() const;
	std::deque<std::string> getServerName() const;
	std::string             getErrorPage(int) const;
	std::string             getClientMaxBodySize() const;
	std::string             getHost() const;
	std::string             getRoot() const;
	std::deque<std::string> getIndex() const;
	std::deque<Location>    getLocation() const;

	// Location getter , int parameter is the index of the container
	std::string             getLocName(int) const;
	std::string             getLocRedirectionUri(int, int) const;
	std::deque<std::string> getLocMethods(int) const;
	std::string             getLocRoot(int) const;
	bool                    getLocAutoindent(int) const;
};

#endif // SERVER_HPP
