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

	static void  _sendAnswer(const std::string &answer, struct epoll_event &event);
	bool         _listenClientResponse(struct epoll_event &event, char *buffer) const;
	std::string  _buildAnswer(int i);
	void         _initServer(void);
	void         _serverLoop(void);
	Application &_getApplicationFromFD(int sockfd);

	public:
	//	Server();
	Server(const std::string &);
	~Server();

	void         startServer(void);
	Application &getRightApplication(const std::pair<std::string, bool> &requestHost);
};

#endif // !SERVER_HPP
