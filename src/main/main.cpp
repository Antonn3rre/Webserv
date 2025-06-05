#include "Server.hpp"
#include <iostream>

int main() {
	std::cout << "\e[33mTEST START SERVER\e[0m" << std::endl;
  try {
	  Server test("conf/defaultWithoutCommentaries.conf");
	  test.startServer();
  } catch (Config::Exception &e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
