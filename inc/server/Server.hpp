#ifndef SERVER_HPP
#define SERVER_HPP

#include "Application.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <cstdio>
#include <string>
#include <sys/epoll.h>
#include <vector>

#define REQUEST_FLAGS  EPOLLIN | EPOLLRDHUP | EPOLLERR
#define RESPONSE_FLAGS EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLOUT

#define MAX_EVENTS 100
#define TIME_OUT   3000

extern int g_sigint;

struct s_cgiSession {
	int                clientFd;
	pid_t              cgiPid;
	int                pipeToCgi;
	int                pipeFromCgi;
	std::string        requestBody;
	std::string        cgiResponse;
	size_t             bytesWrittenToCgi;
	size_t             bytesWrittenToClient;
	struct epoll_event event;
	RequestMessage     request;

	s_cgiSession(int cfd, const RequestMessage &request, struct epoll_event &event)
	    : clientFd(cfd), cgiPid(-1), pipeToCgi(-1), pipeFromCgi(-1), bytesWrittenToCgi(0),
	      bytesWrittenToClient(0), event(event), request(request) {}
};

enum e_status { READING_INPUT, PROCESSING, WRITING_OUTPUT, FINISHED };

struct s_connection {
	int         clientFd;
	std::string bufferRead;
	std::string bufferWrite;
	int         status;
	int         bytesToRead;  // en fonction de contentLength
	size_t      bytesWritten; // pour le send final
	                          //	RequestMessage  request;
	                          //	ResponseMessage response;
	bool chunk;

	s_connection(int clientFd)
	    : clientFd(clientFd), status(READING_INPUT), bytesToRead(-1), bytesWritten(0),
	      chunk(false) {} // NULL a verifier, mettre des pointeurs ?
};

class Server {
	private:
	std::vector<Application>     _applicationList;
	int                          _epollfd;
	std::map<int, Application *> _clientAppMap;

	void _listenClientRequest(int clientfd, unsigned long clientMaxBodySize);
	bool _sendAnswer(s_connection &con, struct epoll_event &ev, const ResponseMessage &);
	// void        _sendAnswer(const std::string &answer, int clientfd);

	void _modifySocketEpoll(int epollfd, int client_fd, int flags);

	bool _evaluateClientConnection(int clientfd, const ResponseMessage &response);
	void _disconnectClient(int clientfd) const;
	void _initServer();
	void _serverLoop();
	bool _checkServerState();
	void _shutdown();

	Application &_getApplicationFromFD(int sockfd) const;

	void _handleActiveCgi(struct epoll_event &event, s_connection *con);
	void _stopWritingToCgi(s_cgiSession *session);
	void _stopReadingFromCgi(s_cgiSession *session);
	void _cleanupCgiSession(s_cgiSession *session);
	void _cleanupConnection(int fd);

	void _clearForNewRequest(int clientFd);
	void _finalizeCgiRead(s_cgiSession *session);
	//	void _resetRequest(int clientfd);

	public:
	//	Server();
	Server(const std::string &);
	~Server();

	int                            getEpollFd() const;
	std::map<int, s_cgiSession *>  cgiSessions;
	std::map<int, s_connection *>  connections;
	std::map<int, ResponseMessage> responseMap;
	std::map<int, RequestMessage>  requestMap;
	void                           startServer();
};

#endif // !SERVER_HPP
