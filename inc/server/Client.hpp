#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <vector>
# include <stdint.h>
# include <netinet/in.h>

class Client {
	public:
		Client();
		int16_t				client_fd;
		std::vector<int>	clientList;
};

#endif // !CLIENT_HPP
