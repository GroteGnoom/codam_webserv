#include "request.hpp"
#include <poll.h>

std::map<std::string, std::string>	get_current_pair(std::map<std::string, std::string> request_info, char *buffer, unsigned int *i, long read_ret) {
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

std::map<std::string, std::string>	get_first_line(std::map<std::string, std::string> request_info, char *buffer, unsigned int *i, long read_ret) {
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

t_request	get_request_info(int socket) {
	char								buffer[300000] = {0};
	long								read_ret;
	unsigned int 						i = 0;
	std::map<std::string, std::string>	request_info;
	std::string							value;
	t_request							request;

	struct pollfd pfd;
	pfd.events = POLL_IN;
	pfd.revents = 0;
	pfd.fd = socket;
	poll(&pfd, 1, -1);
	read_ret = read(socket, buffer, 300000);
	if (read_ret < 0) {
		std::cout << "Failed to read, errno: " << errno << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string whole_request(buffer, read_ret);
	request.whole_request =  whole_request;
	request_info = get_first_line(request_info, buffer, &i, read_ret);
	while (buffer[i] != '\r' && i < read_ret) {
		request_info = get_current_pair(request_info, buffer, &i, read_ret);
		i++;
	}
	i++;
	request.headers = request_info;
	request.body = get_body(buffer, i, read_ret);
	std::cout << "request body: " << request.body << "\n";
	return (request);
}
