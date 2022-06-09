#include <map>
#include <string>
#include <iostream>
#include <unistd.h>

std::map<std::string, std::string>	get_current_pair(char *buffer, int *i);
std::map<std::string, std::string>	get_request_info(int socket);