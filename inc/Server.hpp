#ifndef SERVER_HPP
#define SERVER_HPP

#include <fstream>
#include <iostream>
#include <string>

class Server {
  public:
	Server(char *);
	Server(const Server &);
	Server &operator=(const Server &);
	~Server();

	class FileProblemException : public std::exception {
	  public:
		FileProblemException(const std::string &message);
		const char *what() const throw() { return _errorMessage.c_str(); }
		virtual ~FileProblemException() throw();

	  private:
		std::string _errorMessage;
	};

  private:
	std::string _listen;
	//	std::list<std::string> server_names; // vector
	//	std::map<int, std::string> error_page;
	std::string _host;
	std::string _root;
	//	std::list<std::string> index;
	//	std::list<Location> location;
};

#endif // !SERVER_HPP
