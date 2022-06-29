#include "conf.hpp"
#include "socket.hpp"
#include <iostream>
#include <map>

int main(int argc, char **argv) {
    t_settings settings;
    if (argc != 2) {
        settings = read_conf((char *)"default.conf");
    } else {
		settings = read_conf(argv[1]);
	}
    std::map<int, std::vector<std::string> >   pairs;
    for (size_t i = 0; i < settings.servers.size(); i++) {
        if (pairs.find(settings.servers[i].listen_port) != pairs.end()) {
            pairs.find(settings.servers[i].listen_port)->second.push_back(settings.servers[i].name);
        } else {
            pairs.insert(std::pair<int, std::vector<std::string> >(settings.servers[i].listen_port, std::vector<std::string>(1, settings.servers[i].name)));
        }
    }
    for (std::map<int, std::vector<std::string> >::iterator it = pairs.begin(); it != pairs.end(); it++) {
        if (std::unique(it->second.begin(), it->second.end()) != it->second.end()) {
            std::cout << "Some servers have the same port and server_name\n";
            throw std::exception();
        }
    }
	std::cout << "server started\n";
    listen_to_new_socket(settings);
}
