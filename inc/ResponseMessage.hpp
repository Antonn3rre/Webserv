#ifndef REQUESTMESSAGE_HPP
#define REQUESTMESSAGE_HPP

#include "AMessage.hpp"
#include "StatusLine.hpp"

class RequestMessage : public AMessage {
	private:
	StatusLine _startLine;

	public:
	RequestMessage();
};

#endif
