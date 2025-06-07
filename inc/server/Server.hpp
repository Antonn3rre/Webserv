#ifndef SERVER_HPP
#define SERVER_HPP

#include "Application.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <map>
#include <vector>

#define MAX_EVENTS 1000
#define TIME_OUT   3000

extern int g_sigint;

class Server {
	private:
	std::vector<Application>        _applicationList;
	int                             _epollfd;
	std::map<int, Application *>    _clientAppMap;
	std::map<int, RequestMessage &> _cgiOutputFds;

	void               _listenChunkedRequest(int clientfd, RequestMessage &request,
	                                         unsigned long clientMaxBodySize);
	RequestMessage     _listenClientRequest(int clientfd, unsigned long clientMaxBodySize);
	void               _listenBody(int clientfd, RequestMessage &request, unsigned long sizeLeft);
	static std::string _listenCgiOutput(int outfd);

	bool _handleCgiOutput(int fd, const Config &config);
	bool _acceptNewClient(int eventfd);

	void _sendAnswer(const std::string &answer, int clientfd);

	void _evaluateClientConnection(int clientfd, const ResponseMessage &response);
	void _disconnectClient(int clientfd);

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
