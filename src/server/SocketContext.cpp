#include "SocketContext.hpp"
#include "Application.hpp"

SocketContext::SocketContext(SocketType type, int fd, Application *app)
    : _type(type), _fd(fd), _app(app) {}

SocketContext::~SocketContext() {}

SocketContext::SocketType SocketContext::getType() const { return _type; }

int SocketContext::getFd() const { return _fd; }

Application *SocketContext::getApplication() const { return _app; }
