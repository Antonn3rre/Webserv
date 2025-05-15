#ifndef SERVER_HPP
# define SERVER_HPP

// # include "Config.hpp"
# include <sys/socket.h>
# include <netinet/in.h>

class Server {
	private:
		// Config	_config;
		int		_msocket;
		struct sockaddr_in	_server_addr;

	public:
		Server(void);
		// Server(char *configFile);
		void	startServer(void);
		void	handleClient(void);
};

#endif // SERVER_HPP
