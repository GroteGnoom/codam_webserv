#include "request.hpp"
#include "response.hpp"
#include <fcntl.h>

std::string get_post(t_request request) {
	(void) request;
	std::cout << request.body;

	unsigned int start = 0;
	while (start < request.body.size() - 1) {
		if (request.body[start] == '\n' && request.body[start + 1] == '\r') {
			break;
		}
		start++;
	}
	start += 3;
	unsigned int end = request.body.size() - 1;
	while (end > 0) {
		std::cout << "end: " << end << "\n";
		if (request.body[end] == '\r' && request.body[end - 1] == '\n') {
			break;
		}
		end--;
	}
	std::cout << "size: " << request.body.size() << "\n";
	std::cout << "start: " << start << "\n";
	std::cout << "file: " << request.body.substr(start, end - start)  << "\n";
	std::cout << "saving file!\n";
	int fd = open("tests/post_output_test", O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (fd < 0)
		perror("");
	std::cout << "fd" << fd << "\n";
	write (fd, &request.body[start], end - start);
	close(fd);
	std::cout << "done!\n";
	t_response response;
	response.body = "your file has been saved!\n";
	response.code = 200;
	return response_to_string(response);
}