#ifndef SERVER_HPP
#define SERVER_HPP

#include "Application.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <vector>

#define REQUEST_FLAGS  EPOLLIN | EPOLLRDHUP | EPOLLERR
#define RESPONSE_FLAGS EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLOUT

#define MAX_EVENTS 100
#define TIME_OUT   3000

extern int g_sigint;

struct CgiContext {
	int         pid;
	int         fd_out;
	int         fd_in;
	std::string buffer;
	size_t      body_written;
	std::string body;
};

class Server {
	private:
	std::vector<Application>     _applicationList;
	int                          _epollfd;
	std::map<int, Application *> _clientAppMap;
	std::map<int, CgiContext>    _cgiContexts;

	static void           _listenChunkedRequest(int clientfd, RequestMessage &request,
	                                            unsigned long clientMaxBodySize);
	static RequestMessage _listenClientRequest(int clientfd, unsigned long clientMaxBodySize);
	static void _listenBody(int clientfd, RequestMessage &request, unsigned long sizeLeft);
	void        _sendAnswer(const std::string &answer, struct epoll_event &ev);
	// void        _sendAnswer(const std::string &answer, int clientfd);

	void _modifySocketEpoll(int epollfd, int client_fd, int flags);

	bool _evaluateClientConnection(int clientfd, const ResponseMessage &response);
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
