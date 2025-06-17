#ifndef CGISESSION_HPP
#define CGISESSION_HPP

#include "RequestMessage.hpp"
#include <sys/epoll.h>
#include <unistd.h>

class cgiSession {
	public:
	cgiSession();
	cgiSession(int cfd, const RequestMessage &request, struct epoll_event &event);
	~cgiSession();
	cgiSession &operator=(const cgiSession &rhs);

	// SETTERS
	void setRequestBody(std::string requestbody);
	void setCgiPid(pid_t pid);
	void setPipeToCgi(int fd);
	void setPipeFromCgi(int fd);
	void setTimeStart(time_t time);

	// GETTERS
	int            getClientFd(void) const;
	std::string    getRequestBody(void) const;
	pid_t          getCgiPid(void) const;
	int            getPipeToCgi(void) const;
	int            getPipeFromCgi(void) const;
	time_t         getTimeStart(void) const;
	size_t         bytesWrittenToCgi;
	size_t         bytesWrittenToClient;
	std::string    cgiResponse;
	RequestMessage request;

	private:
	int                _clientFd;
	pid_t              _cgiPid;
	int                _pipeToCgi;
	int                _pipeFromCgi;
	struct epoll_event _event;
	time_t             _time;
};

#endif // !CGISESSION_HPP
