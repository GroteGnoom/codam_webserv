#ifndef RESPONSE_HPP
#define RESPONSE_HPP
struct t_response {
	std::string body;
	int code;
};

std::string get_reponse();
std::string get_reponse_from_page(std::string webpage);
std::string not_found();
#endif
