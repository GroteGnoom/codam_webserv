#include <string>
struct t_settings {
    int listen;
    std::string root;
};

t_settings read_conf(char *conf_file) {
    t_settings settings;
    (void) conf_file;
    return settings;
};
