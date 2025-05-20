#include "Config.hpp"
#include "Header.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Server.hpp"
#include "StatusLine.hpp"
#include <exception>
#include <iostream>
#include <string>

int main() {
	std::cout << std::endl << "\e[35mTESTS WEBSERV\e[0m" << std::endl << std::endl;
	// Message Parsing Test
	{
		std::cout << "\e[33mTEST REQUEST MESSAGE PARSING\e[0m" << std::endl;
		std::string requestStr =
		    "GET /ip HTTP/1.1\r\nHost: httpbin.org\r\n\r\n{\r\n\tblabla\r\n\tasdasd\r\n}";

		try {
			RequestMessage request(requestStr);
			std::cout << request.str();
			std::cout << std::endl << std::endl;
		} catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
		}

		std::cout << "\e[33mTEST RESPONSE MESSAGE PARSING\e[0m" << std::endl;
		std::string responseStr =
		    "HTTP/1.1 200 OK\r\nDate: Mon, 12 May 2025 16:29:56 GMT\r\nContent-Type: "
		    "application/json\r\nContent-Length: 31\r\nConnection: keep-alive\r\nServer: "
		    "gunicorn/19.9.0\r\nAccess-Control-Allow-Origin: "
		    "*\r\nAccess-Control-Allow-Credentials: "
		    "true\r\n\r\n{\r\n\t\"origin\": \"62.210.35.12\"\r\n}";

		try {
			ResponseMessage response(responseStr);
			std::cout << response.str();
			std::cout << std::endl << std::endl;
		} catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
		}
	}
	// Reponse Message Creation
	{
		std::cout << "\e[33mTEST RESPONSE MESSAGE CREATION\e[0m" << std::endl;
		StatusLine      startline("HTTP/1.1", 200, "OK");
		std::string     body("{\n\t\"origin\": \"62.210.35.12\"\n}");
		ResponseMessage response(startline, body);
		response.addHeader(Header("Date", "Mon, 12 May 2025 16:29:56 GMT"));
		response.addHeader(Header("Content-Type", "application/json"));
		response.addHeader(Header("Content-Length", "31"));
		response.addHeader(Header("Connection", "keep-alive"));
		response.addHeader(Header("Server", "gunicorn/19.9.0"));
		response.addHeader(Header("Access-Control-Allow-Origin", "*"));
		response.addHeader(Header("Access-Control-Allow-Credentials", "true"));

		std::cout << response.str();
		std::cout << std::endl << std::endl;
	}
	// Server Start
	{
		// std::cout << "\e[33mTEST START SERVER\e[0m" << std::endl;
		// Server test;
		// test.startServer();
	}
	// Config test
	{
		std::cout << "\e[33mTEST CONFIG\e[0m" << std::endl;
		try {
			Config config("conf/defaultWithoutCommentaries.conf");
			std::cout << config.getErrorPage(503) << std::endl << std::endl;
		} catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
		}
	}
}
