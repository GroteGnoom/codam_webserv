#include "conf.hpp"
#include <iostream>

int main(int argc, char **argv) {
    t_settings settings;
    if (argc != 2) {
        std::cout << "Please provide a configuration file\n"; //TODO use default configuration
		settings.listen_port = 8080;
		settings.root = "/";
        return 0;
    } else {
		settings = read_conf(argv[1]);
	}


}
