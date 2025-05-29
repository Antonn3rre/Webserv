#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Config.hpp"
#include <netinet/in.h>
#include <sys/socket.h>

#define BACKLOG 5

class Application {
	private:
	Config _config;
	int    _lsockfd;

	void        _printAtLaunch(void);
	static bool _listenClientResponse(struct epoll_event &events, char *buffer);
	static void _sendAnswer(const std::string &answer, struct epoll_event &event);
	// void        _serverLoop();
	std::string _buildAnswer(void);

	public:
	//	Application();
	Application(char *configFile);
	Application(std::fstream &file);
	Application(const Application &);
	// ~Application();
	// void startServer(void);

	class ApplicationError : public std::exception {
		public:
		ApplicationError(const std::string &error, const std::string &argument);
		const char *what() const throw();
		virtual ~ApplicationError() throw();

		private:
		std::string _message;
	};

	void initApplication(int epollfd);

	// Config getter
	const Config &getConfig(void) const;
	const int    &getLSockFd(void) const;
};

#endif // APPLICATION_HPP
