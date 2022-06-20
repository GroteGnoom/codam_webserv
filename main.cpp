#include "conf.hpp"
#include "socket.hpp"
#include <iostream>

int main(int argc, char **argv) {
    t_settings settings;
    if (argc != 2) {
        settings = read_conf((char *)"default.conf");
    } else {
		settings = read_conf(argv[1]);
	}
    listen_to_new_socket(settings.servers[0].listen_port, settings);
}

