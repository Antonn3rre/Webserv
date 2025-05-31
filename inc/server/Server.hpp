#ifndef SERVER_HPP
#define SERVER_HPP

#include "Application.hpp"
#include <vector>

#define MAX_EVENTS 1000
#define TIME_OUT   3000

class Server {
	private:
	std::vector<Application>     _applicationList;
	int                          _epollfd;
	std::map<int, Application *> _clientAppMap;

	void        _sendAnswer(const std::string &answer, int clientfd);
	static bool _listenClientRequest(int clientfd, std::string &result,
	                                 unsigned long clientMaxBodySize);
	void        _initServer(void);
	void        _serverLoop(void);

	Application &_getApplicationFromFD(int sockfd) const;

	public:
	//	Server();
	Server(const std::string &);
	~Server();

	void startServer(void);
};

#endif // !SERVER_HPP
