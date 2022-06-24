#pragma once

struct t_response {
	std::string body;
	int 		code;
};

std::string response_to_string(t_response resp);
std::string get_response(std::string body);
std::string get_response_from_page(std::string webpage);
std::string not_found();
