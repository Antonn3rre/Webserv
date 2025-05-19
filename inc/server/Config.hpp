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
	const std::string             &getListen() const;
	const std::deque<std::string> &getServerName() const;
	const std::string             &getErrorPage(int index) const;
	const std::string             &getClientMaxBodySize() const;
	const std::string             &getHost() const;
	const std::string             &getRoot() const;
	const std::deque<std::string> &getIndex() const;
	const std::deque<Location>    &getLocation() const;

	// Location getter , int parameter is the index of the container
	const std::string                 &getLocName(int) const;
	const std::pair<int, std::string> &getLocRedirection(int index) const;
	const std::deque<std::string>     &getLocMethods(int) const;
	const std::string                 &getLocRoot(int) const;
	const bool                        &getLocAutoindent(int) const;

	// Addtionnal getter
	int getNumOfLoc() const;

	private:
	std::string                _listen;
	std::deque<std::string>    _serverName;
	std::map<int, std::string> _errorPages;
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

	void _setDefaultErrorPages();
	void _setDefaultConfig();
};

#endif // !CONFIG_HPP
