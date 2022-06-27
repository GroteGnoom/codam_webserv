#include "request.hpp"
#include "response.hpp"
#include "post.hpp"
#include <fcntl.h>

std::string get_post(t_request request, t_settings settings) {
	t_response		response;
	unsigned int	start = 0;
	unsigned int	end = 0;

	if (!request.body.size()) {
		std::cout << "no body!\n";
		response.body = "";
		response.code = 400;
		return response_to_string(response);
	} else if (request.body.size() > settings.size_string) {
		std::cout << "Body too large!\n";
		response.body = "";
		response.code = 413;
		return response_to_string(response);
	}

	while (start < request.body.size() - 1) {
		if (request.body[start] == '\n' && request.body[start + 1] == '\r') {
			break;
		}
		start++;
	}
	
	start += 3;

	if (request.body.size() >= 3)
		end = request.body.size() - 3;
	while (end > 0) {
		if (request.body[end] == '\r' || request.body[end] == '\n')
			break;
		end--;
	}
	
	std::cout << "saving file!\n";
	int fd = open("tests/post_output_test", O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (fd < 0)
		perror("");
	
	write (fd, &request.body[start], end - start);
	close(fd);
	
	std::cout << "done!\n";
	response.body = "your file has been saved!\r\n<a href=\"/../\">Go to home page</a>\r\n";
	response.code = 200;
	return response_to_string(response);
}
