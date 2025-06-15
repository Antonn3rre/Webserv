#include "cgiSession.hpp"

cgiSession::cgiSession()
    : bytesWrittenToCgi(0), bytesWrittenToClient(0), _clientFd(-1), _cgiPid(-1), _pipeToCgi(-1),
      _pipeFromCgi(-1) {}

cgiSession::cgiSession(int cfd, const RequestMessage &request, struct epoll_event &event)
    : bytesWrittenToCgi(0), bytesWrittenToClient(0), request(request), _clientFd(cfd), _cgiPid(-1),
      _pipeToCgi(-1), _pipeFromCgi(-1), _event(event) {}

cgiSession::~cgiSession(void) {};

cgiSession &cgiSession::operator=(const cgiSession &rhs) {
	if (this != &rhs) {
		bytesWrittenToCgi = rhs.bytesWrittenToCgi;
		bytesWrittenToClient = rhs.bytesWrittenToClient;
		request = rhs.request;
		_clientFd = rhs._clientFd;
		_cgiPid = rhs._cgiPid;
		_pipeToCgi = rhs._pipeToCgi;
		_pipeFromCgi = rhs._pipeFromCgi;
		_event = rhs._event;
		cgiResponse = rhs.cgiResponse;
	}
	return *this;
}

// SETTERS
// void cgiSession::setRequestBody(std::string requestbody) { _requestBody = requestbody; }
void cgiSession::setCgiPid(pid_t pid) { _cgiPid = pid; }
void cgiSession::setPipeToCgi(int fd) { _pipeToCgi = fd; }
void cgiSession::setPipeFromCgi(int fd) { _pipeFromCgi = fd; }

// GETTERS
int cgiSession::getClientFd(void) const { return _clientFd; }
// std::string cgiSession::getRequestBody(void) const { return _requestBody; }
pid_t cgiSession::getCgiPid(void) const { return _cgiPid; }
int   cgiSession::getPipeToCgi(void) const { return _pipeToCgi; }
int   cgiSession::getPipeFromCgi(void) const { return _pipeFromCgi; }
