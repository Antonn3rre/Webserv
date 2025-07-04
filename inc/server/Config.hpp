#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Location.hpp"
#include <fstream>
#include <map>
#include <string>
#include <vector>

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
	const std::string                           &getAddress() const;
	const int                                   &getPort() const;
	const std::vector<std::string>              &getApplicationName() const;
	std::string                                  getErrorPage(unsigned short status) const;
	const std::map<unsigned short, std::string> &getErrorPages() const;
	unsigned long                                getClientMaxBodySize() const;
	const std::string                           &getRoot() const;
	const std::vector<std::string>              &getIndex() const;
	const std::vector<Location>                 &getLocations() const;
	const std::pair<int, std::string>           &getRedirection() const;

	// Location getter , int parameter is the index of the container
	const std::string                 &getLocName(int) const;
	const std::pair<int, std::string> &getLocRedirection(int index) const;
	const std::vector<std::string>    &getLocMethods(int) const;
	const std::vector<std::string>    &getLocIndex(int) const;
	const std::string                 &getLocRoot(int) const;
	const bool                        &getLocAutoindex(int) const;

	// Addtionnal getter
	int getNumOfLoc() const;

	private:
	std::string                           _address;
	int                                   _port;
	std::vector<std::string>              _applicationName;
	std::map<unsigned short, std::string> _errorPages;
	unsigned int                          _clientMaxBodySize;
	std::string                           _root;
	std::vector<std::string>              _index;
	std::vector<Location>                 _locations;
	std::pair<int, std::string>           _redirection;

	void _parseListen(std::string &, std::fstream &);
	void _parseApplicationName(std::string &, std::fstream &);
	void _parseErrorPage(std::string &, std::fstream &);
	void _parseClientMaxSizeBody(std::string &, std::fstream &);
	void _parseRoot(std::string &, std::fstream &);
	void _parseIndex(std::string &, std::fstream &);
	void _parseLocation(std::string &, std::fstream &);
	void _parseRedirection(std::string &, std::fstream &);

	void _setDefaultErrorPages();
	void _setDefaultConfig();
	void _setDefaultLocation();
};

#endif // !CONFIG_HPP
