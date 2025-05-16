#include "utilsParsing.hpp"
#include <cstddef>
#include <fstream>
#include <string>

std::string trim(const std::string &str) {
	size_t start = 0;
	size_t end = str.length() - 1;

	while (start <= end && isspace(str[start]))
		start++;
	while (end > start && isspace(str[end]))
		end--;
	return str.substr(start, end - start + 1);
}

std::string readToken(std::fstream &file) {
	std::string token;
	char        c;
	while (file.get(c)) {
		if (c == '\n' && token == "}")
			return token;
		if (c == '\n' && !token.empty())
			return "";
		if (isspace(c)) {
			if (token.empty())
				continue;
			break; // fin du token
		}
		token += c;
	}
	return token;
}
