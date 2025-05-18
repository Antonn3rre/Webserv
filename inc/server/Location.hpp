#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <deque>
#include <map>
#include <string>

class Location {
	public:
	Location(std::string &, std::fstream &);
	Location(const Location &);
	Location &operator=(const Location &);
	~Location();

	class Exception : public std::exception {
		public:
		virtual const char *what() const throw();
	};

	const std::string             &getName() const;
	const std::string             &getRedirectionUri(int) const;
	const std::deque<std::string> &getMethods() const;
	const std::string             &getRoot() const;
	const bool                    &getAutoindent() const;

	private:
	std::string                _name;
	std::map<int, std::string> _redirection;
	std::deque<std::string>    _methods;
	std::string                _root;
	bool                       _autoindent;

	void _parseRedirection(std::string &, std::fstream &);
	void _parseMethods(std::string &, std::fstream &);
	void _parseRoot(std::string &, std::fstream &);
	void _parseIndent(std::string &, std::fstream &);
};

#endif // !LOCATION_HPP
