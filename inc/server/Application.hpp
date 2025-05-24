#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config.hpp"
#include <netinet/in.h>
#include <sys/socket.h>

#define BACKLOG    5
#define MAX_EVENTS 1000
#define TIME_OUT   3000

class Application {
	private:
	Config _config;
	int    _lsockfd;
	int    _epollfd;

	void _initApplication();
	void _printAtLaunch(void);
	bool _listenClientResponse(struct epoll_event &events, char *buffer);
	void _sendAnswer(std::string answer, struct epoll_event &event);
	// void        _serverLoop();
	std::string _buildAnswer(void);

	public:
	Application();
	Application(char *configFile);
	// void startServer(void);

	class ApplicationError : public std::exception {
		public:
		ApplicationError(const std::string &error, const std::string &argument);
		const char *what() const throw();
		virtual ~ApplicationError() throw();

		private:
		std::string _message;
	};

	// Config getter
	const Config &getConfig(void) const;
};

#endif // SERVER_HPP
