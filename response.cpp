#include <string>
#include "response.hpp"
#include <sstream>

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
	r += resp.body;
	return r;
}

std::string get_reponse() {
	t_response resp;
	std::string body;

	body += "<!DOCTYPE html>\n";
	body += "<html>\n";
	body += "<head>\n";
	body += "<title>Welcome to webserv</title>\n";
	body += "<h1>Joehoe</h1>\n";
	resp.body = body;
	resp.code = 200;
	return response_to_string(resp);
}