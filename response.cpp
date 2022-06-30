#include <string>
#include "response.hpp"
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

std::string not_found() {
	std::string r;

	r += "HTTP/1.1 404 Not Found\n\n\n";
	r += "<style> p.big { font-size: 50px; } </style>\n";
	r += "<p class=\"big\">404 Not Found</p>\n";
	return r;
}

std::string get_code_string(int code) {
	std::map<int, std::string>	cs;
	std::stringstream 			codestr;

	cs[200] = "OK";
	cs[301] = "Moved Permanently";
	cs[404] = "Not Found";
	cs[405] = "Method Not Allowed";

	codestr << code;
	return codestr.str() + " " + cs[code] + "\n";
}

std::string get_redir_response(std::string to) {
	std::string r;

	r += "HTTP/1.1 " + get_code_string(301) ;
	r += "Server: webserv/1.0\n";
	r += "Connection: close\n";
	r += "Location: " + to;
	r += "\n";

	return r;
}
std::string response_to_string(t_response resp, bool html) {
	std::stringstream	length;
	std::string 		r;

	if (resp.body.c_str())
		length << resp.body.size();

	r += "HTTP/1.1 " + get_code_string(resp.code) ;
	r += "Server: webserv/1.0\n";
	if (html)
		r += "Content-Type: text/html\n";
	else 
		r += "Content-Type: text/plain\n";
	r += "Content-Length: " + length.str() + "\n";
	r += "Connection: close\n";
	r += "\n";

	if (resp.body.c_str())
		r += resp.body;
	return r;
}

std::string get_response(std::string body, bool html) {
	t_response resp;

	resp.body = body;
	resp.code = 200;
	return response_to_string(resp, html);
}

std::string get_response_from_page(std::string webpage) {
	std::cout << "get response from page: " << webpage << "\n";
	bool html = 0;
	if (webpage.find(".html") == webpage.size() - 5)
		html = 1;
	t_response			resp;
	std::ostringstream	sstr;

	std::ifstream input_stream(webpage);
	if (!input_stream.is_open()) {
		std::cerr << "could not open webpage\n";
		throw std::exception();
	}
	
    sstr << input_stream.rdbuf();
	resp.body = sstr.str();
	resp.code = 200;
	return response_to_string(resp, html);
}
