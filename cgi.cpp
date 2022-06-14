#include "cgi.hpp"
#include <string>
#include <stdio.h>
std::string get_cgi(t_request request) {
	char buffer[1024];
	std::string result;
    std::string command = "QUERY_STRING=\"first_name=bla&last_name=bloe\" python3 tests/cgi";
    command.append(request.headers["Request-URI"]);
	FILE* pipe = popen(command.c_str(), "r"); //command is passed through the shell
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
	return result;
}
