#include "utilsParsing.hpp"
#include "utilsSpace.hpp"
#include <cstddef>
#include <fstream>
#include <string>

std::string trim(std::string &str) {
	size_t start = 0;
	size_t end = str.length() - 1;

	while (start <= end && isSpace(str[start]))
		start++;
	while (end > start && isSpace(str[end]))
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
		if (isSpace(c)) {
			if (token.empty())
				continue;
			break; // fin du token
		}
		token += c;
	}
	return token;
}
