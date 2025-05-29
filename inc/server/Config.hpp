#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Location.hpp"
#include <vector>
#include <fstream>
#include <map>
#include <string>

class Config {
	public:
	Config(std::fstream &file);
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

	class Finished : public std::exception {
		public:
		virtual const char *what() const throw();
	};

	// Config getter
	const std::string                           &getListen() const;
	const std::string                           &getAddress() const;
	const int                                   &getPort() const;
	const std::vector<std::string>               &getApplicationName() const;
	std::string                                  getErrorPage(unsigned short status) const;
	const std::map<unsigned short, std::string> &getErrorPages() const;
	const std::string                           &getClientMaxBodySize() const;
	const std::string                           &getHost() const;
	const std::string                           &getRoot() const;
	const std::vector<std::string>               &getIndex() const;
	const std::vector<Location>                  &getLocations() const;

	// Location getter , int parameter is the index of the container
	const std::string                 &getLocName(int) const;
	const std::pair<int, std::string> &getLocRedirection(int index) const;
	const std::vector<std::string>     &getLocMethods(int) const;
	const std::string                 &getLocRoot(int) const;
	const bool                        &getLocAutoindent(int) const;

	// Addtionnal getter
	int getNumOfLoc() const;

	private:
	std::string                           _listen;
	std::string                           _address;
	int                                   _port;
	std::vector<std::string>               _applicationName;
	std::map<unsigned short, std::string> _errorPages;
	std::string                           _clientMaxBodySize;
	std::string                           _host;
	std::string                           _root;
	std::vector<std::string>               _index;
	std::vector<Location>                  _locations;

	void _parseListen(std::string &, std::fstream &);
	void _parseApplicationName(std::string &, std::fstream &);
	void _parseErrorPage(std::string &, std::fstream &);
	void _parseClientMax(std::string &, std::fstream &);
	void _parseHost(std::string &, std::fstream &);
	void _parseRoot(std::string &, std::fstream &);
	void _parseIndex(std::string &, std::fstream &);
	void _parseLocation(std::string &, std::fstream &);

	void _setDefaultErrorPages();
	void _setDefaultConfig();
	void _setDefaultLocation();
};

#endif // !CONFIG_HPP
