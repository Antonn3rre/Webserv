#include "Header.hpp"
#include "RequestMessage.hpp"
#include "ResponseMessage.hpp"
#include "Server.hpp"
#include <iostream>
#include <string>

int main() {
	std::cout << std::endl;

	// Message Parsing Test
	// {
	// 	std::string requestStr = "GET /ip HTTP/1.1\nHost: httpbin.org\n\n{\n\tblabla\n\tasdasd\n}";
	// 	std::string responseStr =
	// 	    "HTTP/1.1 200 OK\nDate: Mon, 12 May 2025 16:29:56 GMT\nContent-Type: "
	// 	    "application/json\nContent-Length: 31\nConnection: keep-alive\nServer: "
	// 	    "gunicorn/19.9.0\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Credentials: "
	// 	    "true\n\n{\n\t\"origin\": \"62.210.35.12\"\n} ";
	//
	// 	// Parse the messages
	// 	RequestMessage  request(requestStr);
	// 	ResponseMessage response(responseStr);
	//
	// 	// Print the messages as strings
	// 	std::cout << request.str();
	// 	std::cout << std::endl << std::endl;
	// 	std::cout << response.str();
	// }
	{
		Server test;
		test.startServer();
	}
}
