#include "utilsSpace.hpp"

int isSpace(char c) {
	if ((c >= 9 && c <= 13) || c == ' ')
		return 1;
	return 0;
}

int justSpaces(const std::string &str) {
	return str.find_first_not_of("\t\f\n\r\v ") == std::string::npos;
	// int i = 0;
	// if (!str[i])
	// 	return (1);
	// while (str[i]) {
	// 	if (!isSpace(str[i]))
	// 		return (0);
	// 	i++;
	// }
	// return (1);
}
