#include "request.hpp"
std::string get_body(char *buffer, unsigned int i, long size) {
	std::string body;
	for (;i < size; i++) {
		body.push_back(buffer[i]);
	}
	return body;
}
