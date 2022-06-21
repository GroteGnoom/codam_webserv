#include "cgi.hpp"
#include <string>
#include <stdio.h>
std::string get_cgi(t_request request) {
	char buffer[1024];
	std::string result;

	int			question_mark_pos = request.headers["Request-URI"].find('?');	
	std::string	query = request.headers["Request-URI"].substr(question_mark_pos + 1, request.headers["Request-URI"].size() - question_mark_pos - 1);
    std::string command = "QUERY_STRING=\"" + query + "\" python3 tests";

    command.append(request.headers["Request-URI"].substr(0, question_mark_pos));
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

std::string get_cgi_post(t_request request) {
	//int line1 = request.body.find("\n", 10);
	//request.body = request.body.substr(line1, request.body.size() - line1);
	request.body = request.body.substr(1, request.body.size() - 1);
	setenv("CONTENT_LENGTH", request.headers["Content-Length"].c_str(), 1);
	setenv("CONTENT_TYPE", request.headers["Content-Type"].c_str(), 1);
	int inpipe[2];
	int outpipe[2];
	pipe(inpipe);
	pipe(outpipe);
	if (fork() == 0) {
		dup2(outpipe[0], STDIN_FILENO);
		dup2(inpipe[1], STDOUT_FILENO);
		std::string command = "tests/cgi";
		command.append(request.headers["Request-URI"]);
		std::cerr << "command: " << command << "\n";
		execl(command.c_str(), command.c_str(), NULL);
	}

	close(outpipe[0]);
	close(inpipe[1]);

	//std::cout << "sending whole request: " << request.whole_request << "\n";
	// std::cout << "sending body: " << request.body << "\n";
	write(outpipe[1], request.body.c_str(), request.body.size());
	//write(outpipe[1], request.whole_request.c_str(), request.whole_request.size());

	char buffer[1024];
	int nread = 1;
	while (nread) {
		// std::cout << "in read loop\n";
		nread = read(inpipe[0], buffer, 1023);
		buffer[nread] = 0;
		// std::cout << "buffer: " << buffer << "\n";
		//std::cout << "nread: " << nread << "\n";
	}
	close(outpipe[1]);
	close(inpipe[0]);
	//std::cout << "done\n";
	return "todo";
}
