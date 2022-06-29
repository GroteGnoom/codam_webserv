#include "request.hpp"
#include "response.hpp"
#include "post.hpp"
#include <fcntl.h>

std::string get_filename(std::string body) {
	std::cout << "body: " << body << "\n";
	size_t filename_pos = body.find("filename=");
	if (filename_pos == std::string::npos)
		return "";
	size_t closing_pos = body.find("\"", filename_pos + 11);
	std::string line = body.substr(filename_pos + 10, closing_pos - filename_pos - 10);
	std::cout << "line: " << line << "\n";
	return line;
}

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
	std::cout << request.whole_request.substr(0, 1000) << "\n";
	
	std::cout << "saving file!\n";
	std::string filename = "tests/" + get_filename(request.body);
	int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (fd < 0)
		perror("");
	
	write (fd, &request.body[start], end - start);
	close(fd);
	
	std::cout << "done!\n";
	response.body = "your file has been saved!\r\n<a href=\"/../\">Go to home page</a>\r\n";
	response.code = 200;
	return response_to_string(response);
}
