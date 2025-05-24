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
	Config _config;
	int    _lsockfd;
	int    _epollfd;

	void        _shutdown();
	std::string _buildAnswer(void);

	public:
	Server();
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
	const Config	&getConfig(void) const;
};

#endif // SERVER_HPP
