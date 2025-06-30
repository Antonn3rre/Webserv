#include "Server.hpp"
#include <iostream>

int main(int argc, char **argv) {
	try {
		if (argc > 2) {
			std::cout << "Error : too many arguments\n";
			return 0;
		}
		if (argc == 2) {
			Server test(argv[1]);
			test.startServer();
		} else {
			Server test("conf/default.conf");
			test.startServer();
		}
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
