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

	void                         addHeader(const Header &header);
	std::pair<std::string, bool> getHeaderValue(const std::string &headerName);

	virtual std::string str() const = 0;

	class MessageError : public std::exception {
		public:
		MessageError(const std::string &error, const std::string &argument);
		const char *what() const throw();
		virtual ~MessageError() throw();

		private:
		std::string _message;
	};

	class SyntaxError : public MessageError {
		public:
		SyntaxError(const std::string &error, const std::string &badSyntax);
	};

	class Unsupported : public MessageError {
		public:
		Unsupported(const std::string &name, const std::string &value);
	};

	class InvalidData : public MessageError {
		public:
		InvalidData(const std::string &name, const std::string &value);
	};

	const std::string   &getBody() const;
	std::vector<Header> &getHeaders();

	protected:
	std::map<std::string, std::pair<Header::HeaderType, bool> > _validHeaders;
	std::vector<Header>                                         _headers;
	std::string                                                 _body;

	private:
	void _insertKnownHeader(const std::string &name, Header::HeaderType type, bool isSupported);
	bool _isHeaderValid(const std::string &headerName);
	void _setValidHeaders();
};

#endif
