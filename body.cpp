#include "request.hpp"
//TODO can be removed
std::string get_body(const char *buffer, unsigned int i, long size) {
	std::string body;
	for (;i < size; i++) {
		body.push_back(buffer[i]);
	}
	return body;
}
