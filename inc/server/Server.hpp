#ifndef SERVER_HPP
#define SERVER_HPP

#include "Application.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <vector>

#define MAX_EVENTS 1000
#define TIME_OUT   3000

extern int g_sigint;

struct s_cgiSession {
	int            clientFd;
	pid_t          cgiPid;
	int            pipeToCgi;
	int            pipeFromCgi;
	std::string    requestBody;
	std::string    cgiResponse;
	size_t         bytesWrittenToCgi;
	size_t         bytesWrittenToClient;
	RequestMessage request;

	s_cgiSession(int cfd, const RequestMessage &request)
	    : clientFd(cfd), cgiPid(-1), pipeToCgi(-1), pipeFromCgi(-1), bytesWrittenToCgi(0),
	      bytesWrittenToClient(0), request(request) {}
};

class Server {
	private:
	std::vector<Application>     _applicationList;
	int                          _epollfd;
	std::map<int, Application *> _clientAppMap;

	static void           _listenChunkedRequest(int clientfd, RequestMessage &request,
	                                            unsigned long clientMaxBodySize);
	static RequestMessage _listenClientRequest(int clientfd, unsigned long clientMaxBodySize);
	static void _listenBody(int clientfd, RequestMessage &request, unsigned long sizeLeft);
	void        _sendAnswer(const std::string &answer, int clientfd);

	void _evaluateClientConnection(int clientfd, const ResponseMessage &response);
	void _disconnectClient(int clientfd) const;
	void _initServer();
	void _serverLoop();
	bool _checkServerState();
	void _shutdown();

	Application &_getApplicationFromFD(int sockfd) const;

	void _handleActiveCgi(const struct epoll_event &event);
	void _stopWritingToCgi(s_cgiSession *session);
	void _stopReadingFromCgi(s_cgiSession *session);
	void _cleanupCgiSession(s_cgiSession *session);
	void _cleanupConnection(int fd);

	public:
	//	Server();
	Server(const std::string &);
	~Server();

	int                           getEpollFd() const;
	std::map<int, s_cgiSession *> cgiSessions;
	void                          startServer();
};

#endif // !SERVER_HPP
