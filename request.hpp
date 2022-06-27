#pragma once
#include <map>
#include <string>
#include <iostream>
#include <unistd.h>

struct t_request {
	std::string 						whole_request;
	std::map<std::string, std::string>	headers;
	std::string							body;
	bool								read_once;
	bool								done_reading;
	bool								done;
	bool								cancelled;
	ssize_t								written;
};

std::string get_body(const char *buffer, unsigned int i, long read_ret);
std::map<std::string, std::string>	get_current_pair(char *buffer, int *i);
void	get_request_info(int socket, t_request *request, std::string resp);
