#pragma once
#include <map>
#include <string>
#include <iostream>
#include <unistd.h>

enum request_state {
	RS_START,
	RS_READ_ONCE,
	RS_PROCESSING,
	RS_WRITING,
	RS_DONE,
	RS_CANCELLED,
};

struct t_request {
	std::string 						whole_request;
	std::map<std::string, std::string>	headers;
	std::string							body;
	request_state						state;
	ssize_t								written;
};

std::string get_body(const char *buffer, unsigned int i, long read_ret);
std::map<std::string, std::string>	get_current_pair(char *buffer, int *i);
void	get_request_info(int socket, t_request *request, std::string *resp);
