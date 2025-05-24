#include "Config.hpp"
#include "HandleRequest.hpp"
#include "Header.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Server.hpp"
#include "StatusLine.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

std::string executeCgi(const std::string &uri) {
	if (access(uri.c_str(), F_OK) == -1)
		throw AMessage::InvalidData("cgi, does not exist", uri);
	if (access(uri.c_str(), X_OK) == -1)
		throw AMessage::InvalidData("cgi, does not have authorization to execute", uri);
	int pipefd[2];
	pipe(pipefd);

	int pid = fork();
	if (pid == 0) {
		close(pipefd[0]);
		char **argv = {NULL};
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		execve(uri.c_str(), argv, NULL);
		std::cerr << "execve error" << std::endl;
		exit(EXIT_FAILURE);
	}
	close(pipefd[1]);
	ssize_t     bytesRead;
	std::string output;
	char        buffer[1024];
	bzero(buffer, 1024);
	do {
		bytesRead = read(pipefd[0], buffer, 1024);
		std::string bufStr(buffer);
		output += bufStr.substr(0, bytesRead);
	} while (bytesRead == 1024);
	close(pipefd[0]);
	return output;
}

int main() {
	// std::cout << std::endl << "\e[35mTESTS WEBSERV\e[0m" << std::endl << std::endl;
	// // Message Parsing Test
	// {
	// 	std::cout << "\e[33mTEST REQUEST MESSAGE PARSING\e[0m" << std::endl;
	// 	std::string requestStr = "GET /ip HTTP/1.1\nHost: httpbin.org\n\n{\n\tblabla\n\tasdasd\n}";
	//
	// 	try {
	// 		RequestMessage request(requestStr);
	// 		std::cout << request.str();
	// 		std::cout << std::endl << std::endl;
	// 	} catch (std::exception &e) {
	// 		std::cerr << e.what() << std::endl;
	// 	}
	//
	// 	std::cout << "\e[33mTEST RESPONSE MESSAGE PARSING\e[0m" << std::endl;
	// 	std::string responseStr =
	// 	    "HTTP/1.1 200 OK\nDate: Mon, 12 May 2025 16:29:56 GMT\nContent-Type: "
	// 	    "application/json\nContent-Length: 31\nConnection: keep-alive\nServer: "
	// 	    "gunicorn/19.9.0\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Credentials: "
	// 	    "true\n\n{\n\t\"origin\": \"62.210.35.12\"\n}";
	//
	// 	try {
	// 		ResponseMessage response(responseStr);
	// 		std::cout << response.str();
	// 		std::cout << std::endl << std::endl;
	// 	} catch (std::exception &e) {
	// 		std::cerr << e.what() << std::endl;
	// 	}
	// }
	// // Reponse Message Creation
	// {
	// 	std::cout << "\e[33mTEST RESPONSE MESSAGE CREATION\e[0m" << std::endl;
	// 	StatusLine      startline("HTTP/1.1", 200, "OK");
	// 	std::string     body("{\n\t\"origin\": \"62.210.35.12\"\n}");
	// 	ResponseMessage response(startline, body);
	// 	response.addHeader(Header("Date", "Mon, 12 May 2025 16:29:56 GMT"));
	// 	response.addHeader(Header("Content-Type", "application/json"));
	// 	response.addHeader(Header("Content-Length", "31"));
	// 	response.addHeader(Header("Connection", "keep-alive"));
	// 	response.addHeader(Header("Server", "gunicorn/19.9.0"));
	// 	response.addHeader(Header("Access-Control-Allow-Origin", "*"));
	// 	response.addHeader(Header("Access-Control-Allow-Credentials", "true"));
	//
	// 	std::cout << response.str();
	// 	std::cout << std::endl << std::endl;
	// }
	// Server Start
	{
		// std::cout << "\e[33mTEST START SERVER\e[0m" << std::endl;
		// Server test;
		// test.startServer();
	}
	// CGI execution
	{
		std::cout << "\n\e[33mTEST EXEC CGI\e[0m" << std::endl;
		try {
			std::cout << executeCgi("website/var/www/cgi-bin/helloworld.cgi") << std::endl;
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
		}
	}
	// Config test
	// {
	// 	std::cout << "\e[33mTEST CONFIG\e[0m" << std::endl;
	// 	try {
	// 		Config config("conf/defaultWithoutCommentaries.conf");
	// 		std::cout << config.getErrorPage(503) << std::endl << std::endl;
	// 	} catch (std::exception &e) {
	// 		std::cerr << e.what() << std::endl;
	// 	}
	// }
}
