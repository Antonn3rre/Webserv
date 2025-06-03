#ifndef AMESSAGE_HPP
#define AMESSAGE_HPP

#include "Header.hpp"
#include <exception>
#include <map>
#include <string>
#include <utility>
#include <vector>

class AMessage {
	public:
	AMessage(const std::string &subMessage);
	AMessage(const std::string &body, const std::vector<Header> &headers);

	virtual const std::string &getHttpVersion() const = 0;

	void addHeader(const Header &header);

	void appendChunk(const std::string &chunk);

	std::pair<std::string, bool> getHeaderValue(const std::string &headerName) const;
	const std::string           &getBody() const;
	const std::vector<Header>   &getHeaders() const;
	void                         setBody(const std::string &);

	virtual std::string str() const = 0;

	class MessageError : public std::exception {
		public:
		MessageError(unsigned short status);
		MessageError(unsigned short status, const std::string &error, const std::string &argument);
		virtual ~MessageError() throw();

		const char    *what() const throw();
		unsigned short getStatusCode() const;

		private:
		std::string    _message;
		unsigned short _statusCode;
	};

	protected:
	std::map<std::string, Header> _validHeaders;
	std::vector<Header>           _headers;
	std::string                   _body;

	private:
	void _insertKnownHeader(const std::string &name, Header::HeaderType type, bool isSupported,
	                        bool isDuplicateAllowed);
	bool _isHeaderValid(const std::string &headerName) const;
	void _setValidHeaders();
	void _checkDuplicateHeaders() const;
	void _checkHostHeader() const;
};

#endif
