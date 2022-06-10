#include <string>
std::string response() {
	std::string r;
	r += "HTTP/1.1 200 OK\n";
	r += " Server: nginx/1.21.6\n";
	r += " Date: Fri, 10 Jun 2022 08:14:54 GMT\n";
	r += " Content-Type: text/html\n";
	r += " Content-Length: 615\n";
	r += " Last-Modified: Thu, 09 Jun 2022 12:15:23 GMT\n";
	r += " Connection: keep-alive\n";
	r += " ETag: \"62a1e45b-267\"\n";
	r += " Accept-Ranges: bytes\n";
	r += "\n";
	r += "<!DOCTYPE html>\n";
	r += "<html>\n";
	r += "<head>\n";
	r += "<title>Welcome to webserv</title>\n";
	r += "<style>\n";
	return r;
}
