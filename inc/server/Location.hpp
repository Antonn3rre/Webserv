#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <vector>

class Location {
	public:
	Location(std::string &token, std::fstream &file, unsigned int clientMaxSizeBody);
	Location(const Location &);
	Location &operator=(const Location &);
	~Location();

	class Exception : public std::exception {
		public:
		Exception(const std::string &message);
		const char *what() const throw() { return _errorMessage.c_str(); }
		virtual ~Exception() throw();

		private:
		std::string _errorMessage;
	};

	const std::string                 &getName() const;
	const std::pair<int, std::string> &getRedirection() const;
	const std::vector<std::string>    &getMethods() const;
	const std::vector<std::string>    &getIndex() const;
	const std::string                 &getRoot() const;
	const bool                        &getAutoindex() const;
	const unsigned long               &getClientMaxSizeBody() const;

	void setDefaultMethods();
	void setDefaultIndex(const std::vector<std::string> &);
	void setDefaultRoot(const std::string &);
	void setDefaultRedirection(const std::pair<int, std::string> &);
	void setRedirectionPage(const std::string &);

	private:
	std::string                 _name;
	std::pair<int, std::string> _redirection; // std::pair<>
	std::vector<std::string>    _methods;
	std::vector<std::string>    _index;
	std::string                 _root;
	bool                        _autoindex;
	unsigned long               _clientMaxBodySize;

	void _parseRedirection(std::string &, std::fstream &);
	void _parseMethods(std::string &, std::fstream &);
	void _parseIndex(std::string &, std::fstream &);
	void _parseRoot(std::string &, std::fstream &);
	void _parseIndent(std::string &, std::fstream &);
	void _parseClientMaxSizeBody(std::string &, std::fstream &);
};

#endif // !LOCATION_HPP
