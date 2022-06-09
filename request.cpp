#include "request.hpp"

std::map<std::string, std::string>	get_current_pair(std::map<std::string, std::string> request_info, char *buffer, unsigned int *i) {
	std::string	key;
	std::string	value;
	int			j = 0;

	for (; buffer[j] != ':'; j++) {
		key.push_back(buffer[j]);
	}
	if (buffer[j] == '\r')
		j++;
	j++;
	for (; buffer[j] != '\n'; j++) {
		value.push_back(buffer[j]);
	}
	*i += j;
	request_info.insert(std::pair<std::string, std::string>(key, value));
	return(request_info);
}

std::map<std::string, std::string>	get_request_info(int socket) {
	char								buffer[30000] = {0};
	long								read_ret;
	unsigned int 						i = 0;
	std::map<std::string, std::string>	request_info;
	std::string							value;

	read_ret = read(socket, buffer, 30000);
	if (read_ret < 0) {
		std::cout << "Failed to read, errno: " << errno << std::endl;
		exit(EXIT_FAILURE);
	}
	request_info.insert(std::pair<std::string, std::string>("Method", value));
	request_info.insert(std::pair<std::string, std::string>("Request-URI", value));
	request_info.insert(std::pair<std::string, std::string>("Version", value));
	for (; buffer[i] != '\n'; i++) {
		request_info = get_current_pair(request_info, buffer, &i);
	}
	return (request_info);
}
