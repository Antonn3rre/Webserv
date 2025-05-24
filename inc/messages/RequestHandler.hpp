#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "AMessage.hpp"
#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"

class RequestHandler {
	public:
	static ResponseMessage generateResponse(const Config &config, const RequestMessage &request);

	class RequestError : AMessage::MessageError {
		public:
		RequestError(const std::string &error, const std::string &argument);
		unsigned short getStatusCode() const;

		private:
		unsigned short _statusCode;
	};

	private:
	RequestHandler();

	static std::string _loadFile(const std::string &filename);
	static void        _saveFile(const std::string &filename, const std::string &body);
	static std::string _executeCgi(const std::string &uri);

	static StatusLine  _generateStatusLine(unsigned short status);
	static void        _generateHeaders(ResponseMessage &response, const RequestMessage &request);
	static std::string _generateBody(const RequestMessage &request, unsigned short &status);

	static bool _checkHost(const RequestMessage &request, const std::string &host);

	static std::string _getCompletePath(const std::string &locRoot, const std::string &requestUri);
};

#endif
