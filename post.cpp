#include "request.hpp"
#include <fcntl.h>

std::string get_post(t_request request) {
	(void) request;
	std::cout << request.body;

	unsigned int start = 0;
	while (start < request.body.size() - 1) {
		if (request.body[start] == '\n' && request.body[start + 1] == '\n') {
			break;
		}
		start++;
	}
	std::cout << "saving file!\n";
	int fd = open("post_output_test", O_WRONLY);
	write (fd, &request.body[start], request.body.size() - start);
	return ("your file has been saved!\n");
}
