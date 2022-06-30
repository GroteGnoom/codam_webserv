#include "request.hpp"
#include "response.hpp"
#include "post.hpp"
#include <fcntl.h>

std::string get_str_between(std::string str, std::string d1, std::string d2) {
	size_t start_first = str.find(d1);
	size_t end_first = start_first + d1.size() + 1;
	size_t start_second = str.find(d2, end_first);
	return str.substr(end_first, start_second - end_first);
}

std::string get_filename(std::string body) {
	return get_str_between(body, "filename=", "\""); //TODO filename can have " in it, I guess
}

std::string get_post(t_request request, t_settings settings) {
	t_response		response;
	unsigned int	start = 0;
	unsigned int	end = 0;

	if (!request.body.size()) {
		std::cout << "no body!\n";
		response.body = "";
		response.code = 400;
		return response_to_string(response, 0);
	} else if (request.body.size() > settings.size_string) { //TODO should be multiple 
		std::cout << "Body too large: size is " << request.body.size() << ", maximum is " << settings.size_string << "\n";
		response.body = "";
		response.code = 413;
		return response_to_string(response, 0);
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
	//std::cout << request.whole_request.substr(0, 1000) << "\n";
	//std::cout << "body size: " << request.body.size() << "\n";
	
	std::cout << "saving file!\n";
	std::string filename = "tests/uploads/" + get_filename(request.body);
	int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (fd < 0)
		perror("");
	
	write (fd, &request.body[start], end - start);
	close(fd);
	
	std::cout << "done!\n";
	response.body = "your file has been saved!\r\n<a href=\"/../\">Go to home page</a>\r\n";
	response.code = 200;
	return response_to_string(response, 1);
}
