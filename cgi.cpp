#include "cgi.hpp"
#include <string>
#include <stdio.h>
std::string get_cgi() {
	char buffer[1024];
	std::string result;
	FILE* pipe = popen("QUERY_STRING=\"first_name=bla&last_name=bloe\" python3 tests/cgitest.py", "r"); //command is passed through the shell
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
