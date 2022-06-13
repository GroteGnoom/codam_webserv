#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

struct t_settings {
    int listen_port;
    std::string root;
    std::string cgi_path;
    std::string error_log;
    std::string error_level;
    std::string access_log;
	std::string index;
};

t_settings read_conf(char *conf_file);
std::vector<std::string> tokenize(std::string conf);
