#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

struct t_location {
	std::string location;
	bool autoindex;
};

struct t_server {
	int listen_port;
	std::string root;
	std::string cgi_path;
	std::string index;
	std::string name;
	std::vector<t_location> location;
};

struct t_settings {
	std::string error_log;
	std::string error_level;
	std::string access_log;
	std::vector<t_server> servers;
};

t_settings read_conf(char *conf_file);
std::vector<std::string> tokenize(std::string conf);
