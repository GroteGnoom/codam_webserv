#include <iostream>
#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <map>
#include "socket.hpp"
#include "request.hpp"
#include "response.hpp"
#include "cgi.hpp"
#include "delete.hpp"
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include "post.hpp"
#include "list_files.hpp"

/*
* create a new socket
* returns socket id on success      */

int	create_socket(void) {
	int	socket_fd;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0){
        std::cout << "Failed to create socket, errno: " << errno << std::endl;
		return (EXIT_FAILURE);
	}
	return (socket_fd);
}

/*
* bind a port to a socket
* returns 1 on success      */

int	identify_socket(int socket, int port, sockaddr_in &address) {
	memset((char *)&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(port);
	const int enable = 1;
	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		std::cout << "error! setsockopt(SO_REUSEADDR) failed\n";
		return (EXIT_FAILURE);
	}
	if (bind(socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cout << "Failed to bind to port " << port << ", errno: " << errno << std::endl;	
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
* make the socket prepared to accept connections (make it listen)
* returns 1 on success      */

int	listening_socket(int socket, int backlog) {
	if (listen(socket, backlog) < 0) {
        std::cout << "Failed to listen to socket, errno: " << errno << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
* create a new socket for the first connection request in the queue
* returns new socket id on success      */
int	accept_socket(int socket, sockaddr_in address) {
	int new_socket;
	int	address_len;

	address_len = sizeof(address);
	new_socket = accept(socket, (struct sockaddr *)&address, (socklen_t *)&address_len);
	if (new_socket < 0) {
        std::cout << "Failed to grab connection, errno: " << errno << std::endl;
        std::cout << " errno: " << EWOULDBLOCK << std::endl;
		perror("");
		return (EXIT_FAILURE);
	}
	return (new_socket);
}

int	method_allowed(std::string method, t_settings settings) {
	for (unsigned long i = 0; i < settings.unaccepted_methods.size(); i++) {
		if (method == settings.unaccepted_methods[i])
			return (0);
	}
	return (1);
}

int	listen_to_new_socket(t_settings settings) {
	struct sockaddr_in					address;
	int									backlog = SOMAXCONN;	//how many requests can be backlogged
	t_request							request;
	std::map<std::string, std::string>	request_info;
	std::set<int>						connections;
	int nr_servers = settings.servers.size();
	std::set<int> ports;
	for (int i = 0; i < nr_servers; i++) {
		ports.insert(settings.servers[i].listen_port);
	}
	int nr_ports = ports.size();

	struct pollfd pfd_init = {-1, POLLIN, 0};
	std::vector<struct pollfd> pfd_ports(nr_ports, pfd_init);
	//struct pollfd pfd_conns[SOMAXCONN];

	for (int i = 0; i < nr_ports; i++) {
		pfd_ports[i].fd = create_socket();
		if (pfd_ports[i].fd == EXIT_FAILURE)
			exit(EXIT_FAILURE);
		if (identify_socket(pfd_ports[i].fd, settings.servers[i].listen_port, address) == EXIT_FAILURE)
			exit(EXIT_FAILURE);
		fcntl(pfd_ports[i].fd, F_SETFL, O_NONBLOCK);
		if (listening_socket(pfd_ports[i].fd, backlog) == EXIT_FAILURE)
			exit(EXIT_FAILURE);
	}
	while (1) {
		/*
		unsigned int i = 0;
		for (std::set<int>::iterator iter = connections.begin(); iter != connections.end(); iter++) {
			pfd_conn[i].fd = *iter;
			pfd_conn[i].events = POLLIN;
			pfd_conn[i].revents = 0;
			i++;
		}
		*/
		poll(&*pfd_ports.begin(), nr_ports, -1);
		connections.clear();
		for (int i = 0; i < nr_ports; i++) {
			if (!(pfd_ports[i].revents & POLLIN)) {
				continue;
			}
			pfd_ports[0].revents = 0;
			int	new_conn;
			new_conn = accept_socket(pfd_ports[0].fd, address);
			fcntl(new_conn, F_SETFL, O_NONBLOCK);
			if (new_conn == EXIT_FAILURE)
				exit(EXIT_FAILURE);
			connections.insert(new_conn);
		}

		for (std::set<int>::iterator iter = connections.begin(); iter != connections.end(); iter++) {
			int new_socket = *iter;
			request = get_request_info(new_socket);
			request_info = request.headers;

			if (!request_info["Request-URI"].compare(settings.redir_src))
				request_info["Request-URI"].replace(0, settings.redir_dst.size(), settings.redir_dst);

			std::string webpage;
			if (request_info["Request-URI"].find('.') == std::string::npos) {
				if (settings.servers[0].index.size()) {
					webpage = settings.servers[0].root + request_info["Request-URI"] + settings.servers[0].index;
				} else {
					webpage = settings.servers[0].root + request_info["Request-URI"] + "/index.html";
				}
			} else {
				webpage = settings.servers[0].root + request_info["Request-URI"];
			}
			// std::cout << "page: " << webpage << "\n";
			std::string resp;

			if (request_info["Request-URI"].size() > 1 && request_info["Request-URI"].find(".py") != std::string::npos) {
				if (request.headers["Method"] == "GET") {
					resp = get_cgi(request);
				}
			}
			else try {
				if (request.headers["Method"] == "GET" && method_allowed("GET", settings)) {
					try {
						resp = get_response_from_page(webpage);
					} catch (...) {
						resp = not_found();
					}
				}
				else if (request.headers["Method"] == "DELETE" && method_allowed("DELETE", settings)) {
					resp = get_delete(webpage);
				}
				else if (request.headers["Method"] == "POST" && method_allowed("POST", settings)) {
					resp = get_post(request, settings);
				} else if (!method_allowed(request.headers["Method"], settings)){
					t_response response;
					response.body = "";
					response.code = 405;
					resp = response_to_string(response);
				} else {
					t_response response;
					response.body = "";
					response.code = 501;
					resp = response_to_string(response);
				}
			} catch (...) {
				if (!settings.servers[0].index.size() && request_info["Request-URI"].find('.') == std::string::npos && settings.servers[0].locations[0].autoindex) { //TODO check location. autoindex is now a global setting :(
					resp = list_files(settings.servers[0].root + request_info["Request-URI"]);
				}
				else resp = not_found();
			}
			// std::cout << resp << std::endl;
			write(new_socket, resp.c_str(), resp.size());
			close(new_socket);
		}
	}
}
