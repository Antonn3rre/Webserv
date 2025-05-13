#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <iostream>
#include <fstream>

class Server {
public:
	Server( char * );
	Server(const Server &);
	Server &operator=(const Server &);
	~Server();
	
	class	FileProblemException : public std::exception {
		public :
			FileProblemException(const std::string &);
			virtual const char* what() const throw() {
				return _errorMessage.c_str();
			}
		private :
			std::string _errorMessage;
	};

private:
	std::string listen;
//	std::list<std::string> server_names; // vector
//	std::map<int, std::string> error_page;
	std::string host;
	std::string root;
//	std::list<std::string> index;
//	std::list<Location> location;
};

#endif // !SERVER_HPP
