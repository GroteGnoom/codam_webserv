#pragma once

struct t_response {
	std::string body;
	int 		code;
};

std::string response_to_string(t_response resp, bool html);
std::string get_response(std::string body, bool html);
std::string get_response_from_page(std::string webpage);
std::string not_found();
std::string get_redir_response(std::string to);
