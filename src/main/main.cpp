#include "Server.hpp"
#include <iostream>
int main() {
	std::cout << "\e[33mTEST START SERVER\e[0m" << std::endl;
	Server test("conf/defaultWithoutCommentaries.conf");
	test.startServer();
	return 0;
}
