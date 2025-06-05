#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace std;

int main() {
	char *method = getenv("REQUEST_METHOD");

	if (!method) {
		cout << "No method\n";
	} else if (!strcmp("POST", method)) {
		cout << "POST method not suitable to the script\n";
	} else {
		cout << "Content-type:text/html\r\n\r\n";
		cout << "<html>\n";
		cout << "<head>\n";
		cout << "<title>Hello World - First CGI Program</title>\n";
		cout << "</head>\n";
		cout << "<body>\n";
		cout << "<h2>Hello World! This is my first CGI program</h2>\n";
		cout << "</body>\n";
		cout << "</html>\n";
		sleep(10);
	}
	return 0;
}
