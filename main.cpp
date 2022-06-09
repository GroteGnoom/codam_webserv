#include "conf.hpp"
#include "socket.hpp"
#include <iostream>

int main(int argc, char **argv) {
    t_settings settings;
    if (argc != 2) {
        //TODO use default configuration
		settings.listen_port = 8080;
		settings.root = "/";
    } else {
		settings = read_conf(argv[1]);
	}
    listen_to_new_socket(settings.listen_port);

}
