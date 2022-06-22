#include "delete.hpp"
#include "response.hpp"
#include <cstdio>

std::string get_delete(std::string webpage) {
	std::string resp;
	std::string	deleted_page;

	if (remove(webpage.c_str()) != 0)
		deleted_page = webpage.substr(0, webpage.find_last_of('/')) + "/not_deleted.html";
	else
		deleted_page = webpage.substr(0, webpage.find_last_of('/')) + "/deleted.html";
	resp = get_response_from_page(deleted_page);
	return (resp);
}
