#ifndef SERVER_HPP
#define SERVER_HPP

#include "Application.hpp"
#include "Client.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "cgiSession.hpp"
#include <cstdio>
#include <string>
#include <sys/epoll.h>
#include <vector>

#define REQUEST_FLAGS  EPOLLIN | EPOLLRDHUP | EPOLLERR
#define RESPONSE_FLAGS EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLOUT

#define MAX_EVENTS 100
#define TIME_OUT   3 // en sec

extern int g_sigint;

enum e_status { READING_INPUT, PROCESSING, WRITING_OUTPUT, FINISHED };

struct s_connection {
	int           clientFd;
	std::string   bufferRead;
	std::string   bufferWrite;
	enum e_status status;
	int           bytesToRead;  // en fonction de contentLength
	size_t        bytesWritten; // pour le send final
	                            //	RequestMessage  request;
	                            //	ResponseMessage response;
	bool chunk;

	s_connection()
	    : clientFd(-1), status(READING_INPUT), bytesToRead(-1), bytesWritten(0), chunk(false) {
	} // NULL a verifier, mettre des pointeurs ?

	s_connection(int clientFd)
	    : clientFd(clientFd), status(READING_INPUT), bytesToRead(-1), bytesWritten(0),
	      chunk(false) {} // NULL a verifier, mettre des pointeurs ?
};

class Server {
	private:
	std::vector<Application> _applicationList;
	int                      _epollfd;
	std::map<int, Client>    _clientMap;

	void _listenClientRequest(int clientfd);
	bool _sendAnswer(s_connection &con);

	static void _modifySocketEpoll(int epollfd, int clientfd, int flags);

	bool _evaluateClientConnection(int clientfd, const ResponseMessage &response);
	bool _acceptClientConnection(int currentFd, int &clientfd);
	void _disconnectClient(int clientfd) const;
	bool _initServer();
	void _serverLoop();
	bool _checkServerState();
	void _checkCgiTime();
	void _shutdown();

	Application &_getApplicationFromFD(int sockfd) const;

	void _checkCgiRights(const std::string &uri);
	void _handleActiveCgi(struct epoll_event &event);
	void _stopWritingToCgi(cgiSession &session);
	void _stopReadingFromCgi(cgiSession &session);
	void _cleanupCgiSession(cgiSession &session);
	void _cleanupConnection(int fd);

	void _clearForNewRequest(int clientFd);
	void _finalizeCgiRead(cgiSession &session);
	//	void _resetRequest(int clientfd);

	public:
	//	Server();
	Server(const std::string &);
	~Server();

	int getEpollFd() const;

	std::map<int, cgiSession>      cgiSessions;
	std::map<int, s_connection>    connections;
	std::map<int, ResponseMessage> responseMap;
	std::map<int, RequestMessage>  requestMap;
	void                           startServer();
};

#endif // !SERVER_HPP
