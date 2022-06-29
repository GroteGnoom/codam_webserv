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

void	get_request_info(int socket, t_request *request, std::string *resp) {
	char								buffer[BUFSIZE] = {0};
	long								read_ret;
	std::string							value;
	struct pollfd 						pfd;

	pfd.events = POLLIN | POLLOUT;
	pfd.revents = 0;
	pfd.fd = socket;
	read_ret = 1;

	poll(&pfd, 1, 0);
	//std::cout << POLLIN;
	//if (!(pfd.revents & POLLOUT))
	//	std::cout << "no POLLOUT\n";
	/*
	if ((pfd.revents)) {
		std::cout << "state: " << request->state << "\n";
		std::cout << "poll in: " << POLLIN << "\n";
		std::cout << "poll out: " << POLLOUT << "\n";
		std::cout << "poll hup: " << POLLHUP << "\n";
		std::cout << "poll err: " << POLLERR << "\n";
		std::cout << "poll nval: " << POLLNVAL << "\n";
		std::cout << "poll: connection: " << pfd.revents << "\n";
	}
	*/
	if (pfd.revents & POLLHUP) {
		if (request->state == RS_WRITING && request->written) {
			request->state = RS_DONE;
			std::cout << "found hup, done\n";
			return;
		}
	}
	//std::cout << "checking read and write\n";
	if (request->state == RS_START) {
		if (pfd.revents & POLLIN) {
			read_ret = read(socket, buffer, BUFSIZE);
			std::cout << "did first read\n";
			if (read_ret < 0) {
				request->state = RS_CANCELLED;
				std::cout << "cancelled, ret < 0\n";
				return;
			}
			if (!read_ret) {
				request->state = RS_CANCELLED;
				std::cout << "cancelled, no first read\n";
				return;
			}
			request->state = RS_READ_ONCE;
			request->whole_request += std::string(buffer, buffer + read_ret);
			std::cout << "first read done: " << request->whole_request.size() << "\n";
			//std::cout << "request: " << request->whole_request << "\n";
		}
		return;
	}
	if (request->state == RS_READ_ONCE) {
		//if (!(pfd.revents & POLLIN) && !(pfd.revents & POLLOUT)) {
		//std::cout << "event: " << pfd.revents << "\n";
		if (!(pfd.revents & POLLIN)) {
			//I think this means we've reached EOF
		//if (!(pfd.revents & POLLIN) && (pfd.revents & POLLHUP)) {
			request->state = RS_PROCESSING;
			read_ret = read(socket, buffer, BUFSIZE);
			std::cout << "done reading\n";
			split_up_request(request); //after request is done
			return;
		}
		else if (pfd.revents & POLLIN) {
			//std::cout << "read in\n";
			read_ret = read(socket, buffer, BUFSIZE);
			//std::cout << "did a read\n";

			if (read_ret < 0) {
				//maybe this should just remove the connection?
				//we are not allowed to check errno
				std::cout << "Failed to read after first, errno: " << errno << std::endl;
				perror("Failed to read: ");
				exit(EXIT_FAILURE);
			}
			if (!read_ret) {
				request->state = RS_CANCELLED;
				std::cout << "cancelled\n";
				return;
			}

			request->state = RS_READ_ONCE;
			request->whole_request += std::string(buffer, buffer + read_ret);
			//std::cout << "read done: " << request->whole_request.size() << "\n";
		} /*else {
			std::cout << "poll in: " << POLLIN << "\n";
			std::cout << "poll out: " << POLLOUT << "\n";
			std::cout << "poll hup: " << POLLHUP << "\n";
			std::cout << "poll value: " << pfd.revents << "\n";
			std::cout << "write ready while reading?\n";
		} */
	}
	else if (pfd.revents & POLLOUT) {
		//std::cout << "written before: " << request->written << std::endl;
		// std::cout << "resp size: " << resp->size() << std::endl;
		ssize_t	write_ret = write(socket, resp->c_str() + request->written, resp->size() - request->written);
		if (write_ret < 0) {
			std::cout << "Failed to write, errno: " << errno << std::endl;
			perror("Failed to write: ");
			exit(EXIT_FAILURE);
		}
		request->written += write_ret;
		//std::cout << "written after: " << request->written << std::endl;
		if (request->written == (ssize_t)resp->size()) {
			std::cout << "writing done\n";
			request->state = RS_DONE;
		}
	}
	//std::cout << "done checking read and write\n";
}
