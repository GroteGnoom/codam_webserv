#include "request.hpp"

std::map<std::string, std::string>	get_current_pair(std::map<std::string, std::string> request_info, char *buffer, unsigned int *i) {
	std::string	key;
	std::string	value;
	int			j = *i;

	for (; buffer[j] != ':'; j++) {
		key.push_back(buffer[j]);
	}
	j += 2;
	for (; buffer[j] != '\n'; j++) {
		value.push_back(buffer[j]);
	}
	*i = j;
	request_info.insert(std::pair<std::string, std::string>(key, value));
	key.clear();
	value.clear();
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
	for (; buffer[i] != ' '; i++) {
		value.push_back(buffer[i]);
	}
	i++;
	request_info.insert(std::pair<std::string, std::string>("Method", value));
	value.clear();
	for (; buffer[i] != ' '; i++) {
		value.push_back(buffer[i]);
	}
	i++;
	request_info.insert(std::pair<std::string, std::string>("Request-URI", value));
	value.clear();
	for (; buffer[i] != '\n'; i++) {
		value.push_back(buffer[i]);
	}
	i++;
	request_info.insert(std::pair<std::string, std::string>("Version", value));
	value.clear();
	while (buffer[i] != '\r') {
		request_info = get_current_pair(request_info, buffer, &i);
		i++;
	}
	return (request_info);
}
