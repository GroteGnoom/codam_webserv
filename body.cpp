#include "request.hpp"
std::string get_body(char *buffer, unsigned int i, long size) {
	std::string body;
	std::cout << "size: " << size << "\n";
	for (;i < size; i++) {
		std::cout << "i: " << i << "\n";
		body.push_back(buffer[i]);
	}
	return body;
}
