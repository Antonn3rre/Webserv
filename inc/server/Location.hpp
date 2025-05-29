#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <vector>
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

	const std::string                 &getName() const;
	const std::pair<int, std::string> &getRedirection() const;
	const std::vector<std::string>     &getMethods() const;
	const std::string                 &getRoot() const;
	const bool                        &getAutoindent() const;

	void setDefaultMethods();
	void setDefaultRoot(const std::string &);

	private:
	std::string                 _name;
	std::pair<int, std::string> _redirection; // std::pair<>
	std::vector<std::string>     _methods;
	std::string                 _root;
	bool                        _autoindent;

	void _parseRedirection(std::string &, std::fstream &);
	void _parseMethods(std::string &, std::fstream &);
	void _parseRoot(std::string &, std::fstream &);
	void _parseIndent(std::string &, std::fstream &);
};

#endif // !LOCATION_HPP
