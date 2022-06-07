#include <string>
#include <vector>
#include <iostream>
#include <fstream>

struct t_settings {
    int listen;
    std::string root;
};

t_settings read_conf(char *conf_file);
std::vector<std::string> tokenize(std::string conf);
