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
	value.clear();
	for (; buffer[j] != ' ' && j < read_ret; j++) {
		value.push_back(buffer[j]);
	}
	j++;
	request_info.insert(std::pair<std::string, std::string>("Request-URI", value));
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

void split_up_request(t_request *request) {
	std::map<std::string, std::string>	request_info;
	unsigned int 						i = 0;

	request_info = get_first_line(request_info, request->whole_request.c_str(), &i, request->whole_request.size());
	while (request->whole_request[i] != '\r' && i < request->whole_request.size()) {
		request_info = get_current_pair(request_info, request->whole_request.c_str(), &i, request->whole_request.size());
		i++;
	}
	i++;

	request->headers = request_info;

	if (i < request->whole_request.size())
		request->body = request->whole_request.substr(i, request->whole_request.size() - i);
}

void	get_request_info(int socket, t_request *request, std::string resp) {
	char								buffer[BUFSIZE] = {0};
	long								read_ret;
	std::string							value;
	struct pollfd 						pfd;

	//for POLL_HUP
	//man literally says: This flag is
	//output only, and ignored if present in the input events
	// but that's not true!
	//We also need POLL_OUT. That's just a rule in the subject, checking for read and write needs to be done at the same time

	pfd.events = POLL_IN | POLL_HUP | POLL_OUT;
	pfd.revents = 0;
	pfd.fd = socket;
	read_ret = 1;

	poll(&pfd, 1, 0);
	//std::cout << "poll\n";
	if (!request->done_reading) {
		if (!(pfd.revents & POLL_IN)) {
			if (request->read_once) {
				//I think this means we've reached EOF
				request->done_reading = true;
				std::cout << "done\n";
				split_up_request(request); //after request is done
			}
			return;
		}
		read_ret = read(socket, buffer, BUFSIZE);

		if (read_ret < 0) {
			//maybe this should just remove the connection?
			//we are not allowed to check errno
			std::cout << "Failed to read, errno: " << errno << std::endl;
			perror("Failed to read: ");
			exit(EXIT_FAILURE);
		}
		if (!read_ret) {
			request->cancelled = true;
			std::cout << "cancelled\n";
			return;
		}

		request->read_once = true;
		request->whole_request += std::string(buffer, buffer + read_ret);
	}
	else {
		if (!(pfd.revents & POLL_OUT)) {
			request->written += write(socket, resp.c_str() + request->written, resp.size() - request->written);
			if (request->written == (ssize_t)resp.size()) {
				request->done = true;
			}
		}
	}
}
