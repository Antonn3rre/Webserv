#ifndef SERVER_HPP
# define SERVER_HPP

// # include "Config.hpp"
# include "Client.hpp"
# include <netinet/in.h>

# define BACKLOG 5

class Server {
	private:
		// Config	_config;
		int		_msocket;
		struct sockaddr_in	_server_addr;
		Client				_client;

	public:
		Server(void);
		// Server(char *configFile);
		void	startServer(void);
		void	handleClient(void);
};

#endif // SERVER_HPP
