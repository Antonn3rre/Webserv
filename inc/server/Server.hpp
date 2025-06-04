#ifndef SERVER_HPP
#define SERVER_HPP

#include "Application.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <vector>

#define MAX_EVENTS 1000
#define TIME_OUT   3000

int g_sigint = 0;

class Server {
	private:
	std::vector<Application>     _applicationList;
	int                          _epollfd;
	std::map<int, Application *> _clientAppMap;

	static void           _listenChunkedRequest(int clientfd, RequestMessage &request,
	                                            unsigned long clientMaxBodySize);
	static RequestMessage _listenClientRequest(int clientfd, unsigned long clientMaxBodySize);
	void                  _sendAnswer(const std::string &answer, int clientfd);

	void _evaluateClientConnection(int clientfd, const ResponseMessage &response);
	void _disconnectClient(int clientfd) const;
	void _initServer();
	void _serverLoop();
	bool _checkServerState();
	void _shutdown();

	Application &_getApplicationFromFD(int sockfd) const;

	public:
	//	Server();
	Server(const std::string &);
	~Server();

	void startServer();
};

#endif // !SERVER_HPP
