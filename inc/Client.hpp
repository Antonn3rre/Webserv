#ifndef CLIENT_HPP
# define CLIENT_HPP

class Client {
	private:
		int					_client_socket_fd;
		struct sockaddr_in	_client_addr;
		socklen_t			_client_addr_size;
	public:
		Client();
};

#endif // !CLIENT_HPP
