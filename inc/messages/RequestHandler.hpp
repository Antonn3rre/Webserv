#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "Config.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusLine.hpp"
#include <vector>

class RequestHandler {
	public:
	static ResponseMessage generateResponse(const Config &config, const RequestMessage &request);

	static ResponseMessage generateErrorResponse(const Config &config, unsigned short status);

	private:
	RequestHandler();

	static ResponseMessage _createResponse(const Config &config, RequestMessage &request,
	                                       std::pair<int, std::string> &handled);

	static StatusLine  _generateStatusLine(unsigned short status);
	static void        _generateHeaders(ResponseMessage &response, const RequestMessage &request,
	                                    unsigned short status);
	static std::string _generateBody(const RequestMessage &request, unsigned short &status,
	                                 const Config &config);
	static std::string _generateErrorBody(unsigned short status, const Config &config);
	static void        _generateErrorHeaders(ResponseMessage &response);

	static bool _checkHostHeader(const RequestMessage &request, const std::string &host);

	static void _addConnectionHeader(const RequestMessage &request, ResponseMessage &response);
	static void _addContentLengthHeader(ResponseMessage &response);
	static void _addContentTypeHeader(const RequestMessage &request, ResponseMessage &response,
	                                  unsigned short status);
	static void _addDateHeader(ResponseMessage &response);
	static std::string _getTime();

	static const Location &_findURILocation(const std::vector<Location> &locations,
	                                        const std::string           &uri);

	static std::string _getCompletePath(const Config &config, const std::string &requestUri);
};

#endif
