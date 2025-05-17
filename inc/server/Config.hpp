#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Location.hpp"
#include <deque>
#include <map>
#include <string>

class Config {
	public:
	Config(const std::string &configFile);
	Config(const Config &);
	Config &operator=(const Config &);
	~Config();

	class Exception : public std::exception {
		public:
		Exception(const std::string &message);
		const char *what() const throw() { return _errorMessage.c_str(); }
		virtual ~Exception() throw();

		private:
		std::string _errorMessage;
	};

	// Config getter
	std::string             getListen() const;
	std::deque<std::string> getServerName() const;
	std::string             getErrorPage(int) const;
	std::string             getClientMaxBodySize() const;
	std::string             getHost() const;
	std::string             getRoot() const;
	std::deque<std::string> getIndex() const;
	std::deque<Location>    getLocation() const;

	// Location getter , int parameter is the index of the container
	std::string             getLocName(int) const;
	std::string             getLocRedirectionUri(int, int) const;
	std::deque<std::string> getLocMethods(int) const;
	std::string             getLocRoot(int) const;
	bool                    getLocAutoindent(int) const;

	private:
	std::string                _listen;
	std::deque<std::string>    _serverName;
	std::map<int, std::string> _errorPage;
	std::string                _clientMaxBodySize;
	std::string                _host;
	std::string                _root;
	std::deque<std::string>    _index;
	std::deque<Location>       _location;

	void _parseListen(std::string &, std::fstream &);
	void _parseServerName(std::string &, std::fstream &);
	void _parseErrorPage(std::string &, std::fstream &);
	void _parseClientMax(std::string &, std::fstream &);
	void _parseHost(std::string &, std::fstream &);
	void _parseRoot(std::string &, std::fstream &);
	void _parseIndex(std::string &, std::fstream &);
	void _parseLocation(std::string &, std::fstream &);
};

#endif // !CONFIG_HPP
