#include "conf.hpp"
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Please provide a configuration file\n"; //TODO use default configuration
        return 0;
    }
    t_settings settings = read_conf(argv[1]);

}
