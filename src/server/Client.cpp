#include "Client.hpp"
#include "Application.hpp"
#include "Server.hpp"

Client::Client()
    : _clientfd(-1), _status(READING_INPUT), _bytesToRead(-1), _bytesWritten(0), _chunk(false) {}

Client::Client(int clientfd)
    : _clientfd(clientfd), _status(READING_INPUT), _bytesToRead(-1), _bytesWritten(0),
      _chunk(false) {}

Client::~Client(){};

Client &Client::operator=(const Client &rhs) {
	if (this != &rhs) {
		_clientfd = rhs._clientfd;
		_status = rhs._status;
		_bytesToRead = rhs._bytesToRead;
		_bytesWritten = rhs._bytesWritten;
		_chunk = rhs._chunk;
	}
	return *this;
}

void Client::setApplication(Application *application) { _application = application; }

Application &Client::getApplication() const { return *_application; }
int          Client::getClientfd(void) const { return _clientfd; }
