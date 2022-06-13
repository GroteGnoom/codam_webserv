#include <string>
#include "response.hpp"
#include <sstream>
#include <iostream>
#include <fstream>

std::string not_found() {
	std::string r;
	r += "HTTP/1.1 404 Not Found\n\n\n";
	r += "<style> p.big { font-size: 50px; } </style>\n";
	r += "<p class=\"big\">404 Not Found</p>\n";
	return r;
}

std::string response_to_string(t_response resp) {
	std::stringstream code;
	code << resp.code;
	std::stringstream length;
	length << resp.body.size();
	std::string r;
	r += "HTTP/1.1 " + code.str() + " OK\n";
	r += "Server: webserv/1.0\n";
	//r += "Date: Fri, 10 Jun 2022 08:14:54 GMT\n";
	r += "Content-Type: text/html\n";
	r += "Content-Length: " + length.str() + "\n";
	//r += "Last-Modified: Thu, 09 Jun 2022 12:15:23 GMT\n";
	r += "Connection: keep-alive\n";
	//r += "ETag: \"62a1e45b-267\"\n";
	//r += "Accept-Ranges: bytes\n";
	r += "\n";
	std::cout << "body:" << resp.body << "\n";
	r += resp.body;
	return r;
}

std::string get_reponse() {
	t_response resp;
	std::string body;

	//body += "<!DOCTYPE html>\n";
	//body += "<html>\n";
	//body += "<head>\n";
	//body += "<title>Welcome to webserv</title>\n";
	//body += "<h1>Joehoe</h1>\n";
	body += "hello!\n";

	resp.body = body;
	resp.code = 200;
	return response_to_string(resp);
}

std::string get_reponse_from_page(std::string webpage) {
	t_response resp;

	std::ifstream input_stream(webpage);
	if (!input_stream.is_open()) {
		std::cerr << "could not open webpage\n";
		throw std::exception();
	}
	std::ostringstream sstr;
    sstr << input_stream.rdbuf();
	//std::cout << "body string:" << sstr.str() << "\n";
	resp.body = sstr.str();
	resp.code = 200;
	return response_to_string(resp);
}
