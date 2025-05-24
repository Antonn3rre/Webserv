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
		std::cout << "\e[33mTEST START SERVER\e[0m" << std::endl;
		Server test;
		test.startServer();
	}
	// CGI execution
	{}
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
