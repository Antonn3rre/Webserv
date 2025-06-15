#pragma once
#include "Application.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include <sys/epoll.h>
#include <unistd.h>

class Client {
	public:
	Client();
	Client(int clientFd);
	~Client();
	Client &operator=(const Client &rhs);

	// SETTERS
	void setApplication(Application *application);

	// GETTERS
	int          getClientfd(void) const;
	Application &getApplication() const;
	size_t       bytesWrittenToClient;

	private:
	RequestMessage  _request;
	ResponseMessage _reponse;

	int          _clientfd;
	std::string  _bufferRead;
	std::string  _bufferWrite;
	int          _status;
	int          _bytesToRead;  // en fonction de contentLength
	size_t       _bytesWritten; // pour le send final
	Application *_application;
	bool         _chunk;
};
