#include "request.hpp"
#include <poll.h>

std::map<std::string, std::string>	get_current_pair(std::map<std::string, std::string> request_info, const char *buffer, unsigned int *i, long read_ret) {
	std::string		key;
	std::string		value;
	unsigned int	j = *i;

	for (; buffer[j] != ':' && j < read_ret; j++) {
		key.push_back(buffer[j]);
	}
	j += 2;
	for (; buffer[j] != '\r' && j < read_ret; j++) {
		value.push_back(buffer[j]);
	}
	j++;
	*i = j;
	request_info.insert(std::pair<std::string, std::string>(key, value));
	std::cout << key << ": " << value << "\n";
	key.clear();
	value.clear();
	return(request_info);
}

std::map<std::string, std::string>	get_first_line(std::map<std::string, std::string> request_info, const char *buffer, unsigned int *i, long read_ret) {
	std::string		value;
	unsigned int	j = *i;
	
	for (; buffer[j] != ' ' && j < read_ret; j++) {
		value.push_back(buffer[j]);
	}
	j++;
	request_info.insert(std::pair<std::string, std::string>("Method", value));
	std::cout << "Method: " << value << std::endl;
	value.clear();
	for (; buffer[j] != ' ' && j < read_ret; j++) {
		value.push_back(buffer[j]);
	}
	j++;
	request_info.insert(std::pair<std::string, std::string>("Request-URI", value));
	std::cout << "Request-URI: " << value << std::endl;
	value.clear();
	for (; buffer[j] != '\r' && j < read_ret; j++) {
		value.push_back(buffer[j]);
	}
	j += 2;
	request_info.insert(std::pair<std::string, std::string>("Version", value));
	value.clear();
	*i = j;
	return (request_info);
}

#define BUFSIZE 1024

t_request	get_request_info(int socket) {
	char								buffer[BUFSIZE] = {0};
	long								read_ret;
	unsigned int 						i = 0;
	std::map<std::string, std::string>	request_info;
	std::string							value;
	t_request							request;

	bool read_once = 0;
	struct pollfd pfd;
	pfd.events = POLL_IN | POLL_HUP;
	pfd.revents = 0;
	pfd.fd = socket;
	read_ret = 1;
	while(read_ret) {
		poll(&pfd, 1, -1);
		if (!(pfd.revents & POLL_IN)) {
			if (read_once)
				//I think this means we've reached EOF
				break;
			continue;
		}
		read_ret = read(socket, buffer, BUFSIZE);
		std::cout << "read: " << read_ret << "\n";
		if (read_ret < 0) {
			std::cout << "Failed to read, errno: " << errno << std::endl;
			exit(EXIT_FAILURE);
		}
		read_once = 1;
		request.whole_request += std::string(buffer, buffer + read_ret);
		std::cout << "whole request size: " << request.whole_request.size() << "\n";
		//I think this never happens
		if (!read_ret)
			break;
		
	}
	request_info = get_first_line(request_info, request.whole_request.c_str(), &i, read_ret);
	while (request.whole_request[i] != '\r' && i < request.whole_request.size()) {
		request_info = get_current_pair(request_info, request.whole_request.c_str(), &i, read_ret);
		i++;
	}
	i++;
	//std::cout << "whole request: " << request.whole_request << "****end of whole request\n";
	request.headers = request_info;
	std::cout << "i: " << i;
	std::cout << "whole size: " << request.whole_request.size() << "\n";
	std::cout << "difference: " << request.whole_request.size() - i << "\n";
	if (i < request.whole_request.size() )
		request.body = request.whole_request.substr(i, request.whole_request.size() - i);
	//request.body = get_body(request.whole_request.c_str(), i, read_ret);
	//std::cout << "request body: " << request.body << "****end of request body\n";
	return (request);
}
