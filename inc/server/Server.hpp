#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Config.hpp"
#include <netinet/in.h>
#include <sys/socket.h>

#define BACKLOG 5

class Server {
	private:
	Config             _config;
	int                _msocket;
	struct sockaddr_in _serverAddr;
	Client             _client;
	fd_set             _readfds;
	size_t             _valread;
	int                _maxfd;
	int                _sd;
	int                _activity;

	public:
	Server();
	Server(char *configFile);
	void        startServer(void);
	void        handleClient(void);
	void        handleMessage(void);
	std::string buildAnswer();

	class ServerError : public std::exception {
		public:
		ServerError(const std::string &error, const std::string &argument);
		const char *what() const throw();
		virtual ~ServerError() throw();

		private:
		std::string _message;
	};

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
	const std::string                 &getLocName(int) const;
	const std::pair<int, std::string> &getLocRedirection(int) const;
	const std::deque<std::string>     &getLocMethods(int) const;
	const std::string                 &getLocRoot(int) const;
	const bool                        &getLocAutoindent(int) const;

	// additionnal getter
	unsigned int getNumOfLoc() const;
};

#endif // SERVER_HPP
