#pragma once
#include <map>
#include <string>
#include <iostream>
#include <unistd.h>

struct t_request {
	std::map<std::string, std::string> headers;
	std::string body;
};

std::string get_body(char *buffer, unsigned int i, long read_ret);
std::map<std::string, std::string>	get_current_pair(char *buffer, int *i);
t_request get_request_info(int socket);

