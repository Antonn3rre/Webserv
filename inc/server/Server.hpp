#ifndef SERVER_HPP
# define SERVER_HPP

// # include "Config.hpp"
# include "Client.hpp"
# include <netinet/in.h>
# include <string>
# include <map>
# include <sys/select.h>

# define BACKLOG 5

class Server {
	private:
		// Config	_config;
		int		_msocket;
		struct sockaddr_in	_server_addr;
		Client				_client;
		fd_set				_readfds;
		size_t				_valread;
		int					_maxfd;
		int					_sd;
		int					_activity;

	public:
		Server(void);
		// Server(char *configFile);
		void		startServer(void);
		void		handleClient(void);
		void		handleMessage(void);
		std::string	buildAnswer(std::string statuscode, std::string statusmsg, std::map<std::string, std::string> headers, std::string body,std::string mimetype);
};

#endif // SERVER_HPP
