#ifndef SERVER_HPP
#define SERVER_HPP

#include "Application.hpp"
#include <vector>

#define MAX_EVENTS 1000
#define TIME_OUT   3000

class Server {
	private:
	std::vector<Application> _applicationList;
	int                      _epollfd;
	void                     _sendAnswer(std::string answer, struct epoll_event &event);
	bool                     _listenClientResponse(struct epoll_event &event, char *buffer);
	std::string              _buildAnswer();
	void                     _initServer(void);
	void                     _serverLoop(void);

	public:
	Server();
	~Server();

	void startServer(void);
};

#endif // !SERVER_HPP
