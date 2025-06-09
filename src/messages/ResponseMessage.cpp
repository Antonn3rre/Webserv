#include "ResponseMessage.hpp"
#include <ctime>
#include <sstream>

ResponseMessage::ResponseMessage(const std::string &message)
    : AMessage(
          message.substr(message.find("\r\n") + 2, message.length() - message.find("\r\n") - 2)),
      _startLine(message.substr(0, message.find("\r\n"))) {}

ResponseMessage::ResponseMessage(const StatusLine &statusLine, const std::string &body)
    : AMessage(body, std::vector<Header>()), _startLine(statusLine) {}

const std::string &ResponseMessage::getHttpVersion() const { return _startLine.getHttpVersion(); }

unsigned short ResponseMessage::getStatusCode() const { return _startLine.getStatusCode(); }

const std::string &ResponseMessage::getReasonPhrase() const { return _startLine.getReasonPhrase(); }

void ResponseMessage::addSessionCookieHeader(const std::string &name, const std::string &value) {
	addHeader(Header("Set-Cookie", name + "=" + value));
}

void ResponseMessage::addPermanentCookieHeader(const std::string &name, const std::string &value) {
	addHeader(Header("Set-Cookie", name + "=" + value + "; Max-Age=2592000"));
}

void ResponseMessage::addDateHeader() { addHeader(Header("Date", _getTime())); }

void ResponseMessage::addContentLengthHeader() {
	std::ostringstream lengthStream;
	lengthStream << getBody().length();
	addHeader(Header("Content-Length", lengthStream.str()));
}

std::string ResponseMessage::str() const {
	std::string str = _startLine.str();
	str += "\r\n";
	for (std::vector<Header>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		str += (*it).str();
		str += "\r\n";
	}
	str += "\r\n";
	str += _body;

	return str;
}

std::string ResponseMessage::_getTime() {
	time_t   timeStamp = time(NULL);
	std::tm *dateTime = std::gmtime(&timeStamp);
	mktime(dateTime);
	std::string timeStr;

	// Week day
	switch (dateTime->tm_wday - 1) // -1 offset because wtf
	{
		case 0:
			timeStr += "Mon";
			break;
		case 1:
			timeStr += "Tue";
			break;
		case 2:
			timeStr += "Wed";
			break;
		case 3:
			timeStr += "Thu";
			break;
		case 4:
			timeStr += "Fri";
			break;
		case 5:
			timeStr += "Sat";
			break;
		case 6:
			timeStr += "Sun";
			break;
	}
	timeStr += ", ";

	std::ostringstream sstream;

	// Month day
	sstream << dateTime->tm_mday;
	timeStr += sstream.str() + " ";
	sstream.str("");

	// Month
	switch (dateTime->tm_mon) {
		case 0:
			timeStr += "Jan";
			break;
		case 1:
			timeStr += "Feb";
			break;
		case 2:
			timeStr += "Mar";
			break;
		case 3:
			timeStr += "Apr";
			break;
		case 4:
			timeStr += "May";
			break;
		case 5:
			timeStr += "Jun";
			break;
		case 6:
			timeStr += "Jul";
			break;
		case 7:
			timeStr += "Aug";
			break;
		case 8:
			timeStr += "Sep";
			break;
		case 9:
			timeStr += "Oct";
			break;
		case 10:
			timeStr += "Nov";
			break;
		case 11:
			timeStr += "Dec";
			break;
	}
	timeStr += " ";

	// Year
	sstream << dateTime->tm_year + 1900 << " ";
	// Hour
	sstream << dateTime->tm_hour << ":" << dateTime->tm_min << ":" << dateTime->tm_sec << " GMT";
	timeStr += sstream.str();

	return timeStr;
}
