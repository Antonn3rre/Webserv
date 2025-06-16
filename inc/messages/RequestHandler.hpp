#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <exception>
#include <vector>

class RequestHandler {
	public:
	static ResponseMessage generateResponse(const Config &config, const RequestMessage &request,
	                                        int clientFd);

	static ResponseMessage generateErrorResponse(const Config &config, unsigned short status);
	static const Location &findURILocation(const std::vector<Location> &locations,
	                                       const std::string           &uri);

	static void       generateHeaders(ResponseMessage &response, const RequestMessage &request,
	                                   unsigned short status);
	static StatusLine generateStatusLine(unsigned short status);

	class CgiRequestException : public std::exception {
		public:
		RequestMessage request;
		int            clientFd;
		std::string    uri;
		Config         config;

		CgiRequestException(const RequestMessage &request, int fd, std::string uri,
		                    const Config &config)
		    : request(request), clientFd(fd), uri(uri), config(config) {}
		virtual ~CgiRequestException() throw() {}
	};

	private:
	RequestHandler();

	static ResponseMessage _createResponse(const Config &config, RequestMessage &request,
	                                       std::pair<int, std::string> &handled);

	//	static StatusLine  generateStatusLine(unsigned short status);
	//	static void        generateHeaders(ResponseMessage &response, const RequestMessage
	//&request, 	                                    unsigned short status);
	static std::string _generateBody(const RequestMessage &request, unsigned short &status,
	                                 const Config &config, int clientFd);
	static std::string _generateErrorBody(unsigned short status, const Config &config);
	static void        _generateErrorHeaders(ResponseMessage &response);

	static bool _checkMethods(const std::vector<std::string> &methods,
	                          const std::string              &requestMethod);
	static bool _checkHostHeader(const RequestMessage &request, const std::string &host);

	static void _addConnectionHeader(const RequestMessage &request, ResponseMessage &response);
	static void _addContentTypeHeader(const RequestMessage &request, ResponseMessage &response,
	                                  unsigned short status);

	static std::string _getCompletePath(const Config &config, const std::string &requestUri);
};

#endif
