#ifndef RESPONSE_HPP
#define RESPONSE_HPP
struct t_response {
	std::string body;
	int code;
};

std::string get_reponse();
#endif
