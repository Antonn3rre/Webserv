#pragma once

#include "Application.hpp"

class SocketContext {
	public:
	enum SocketType {
		LISTEN,
		CLIENT,
	};

	SocketContext(SocketType type, int fd, Application *app);
	~SocketContext();

	SocketType   getType() const;
	int          getFd() const;
	Application *getApplication() const;

	protected:
	SocketType   _type;
	int          _fd;
	Application *_app;
};
