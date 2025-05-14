#ifndef REQUESTMESSAGE_HPP
#define REQUESTMESSAGE_HPP

#include "AMessage.hpp"
#include "RequestLine.hpp"

class RequestMessage : public AMessage {
	private:
	RequestLine _startLine;

	public:
	RequestMessage();
};

#endif
