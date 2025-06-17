#include "Server.hpp"
#include <iostream>

int main() {
	try {
		Server test("conf/default.conf");
		test.startServer();
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
