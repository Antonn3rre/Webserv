#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config.hpp"
#include <netinet/in.h>
#include <sys/socket.h>

#define BACKLOG    5
#define MAX_EVENTS 1000
#define TIME_OUT   3000

class Server {
	private:
	Config      _config;
	int         _lsockfd;
	int         _epollfd;
	std::string _buildAnswer(void);

	public:
	//	Server();
	Server(char *configFile);
	void startServer(void);
	void handleClients(void);
	void handleMessage(void);

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
