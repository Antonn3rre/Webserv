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

	void _printAtLaunch(void);

	public:
	Application(char *configFile);
	Application(std::fstream &file);
	Application(const Application &);

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
