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
#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

int	listen_to_new_socket(int port, t_settings settings) {
	int									server_socket = create_socket();
	struct sockaddr_in					address;
	int									backlog = 1000;	//how many requests can be backlogged
	int									new_socket;
	t_request							request;
	std::map<std::string, std::string>	request_info;

	struct pollfd pfd;
	if (server_socket == EXIT_FAILURE)
		exit(EXIT_FAILURE);
	if (identify_socket(server_socket, port, address) == EXIT_FAILURE)
		exit(EXIT_FAILURE);
	fcntl(server_socket, F_SETFL, O_NONBLOCK);
	if (listening_socket(server_socket, backlog) == EXIT_FAILURE)
		exit(EXIT_FAILURE);
	pfd.events = POLLIN;
	pfd.revents = 0;
	pfd.fd = server_socket;
	while (1) {
		poll(&pfd, 1, 10);
		if (!(pfd.revents & POLLIN)) {
			continue;
		}
		pfd.revents = 0;
		new_socket = accept_socket(server_socket, address);
		fcntl(new_socket, F_SETFL, O_NONBLOCK);
		if (new_socket == EXIT_FAILURE)
			exit(EXIT_FAILURE);
		request = get_request_info(new_socket);
		request_info = request.headers;
		std::string webpage;
		if (request_info["Request-URI"].find('.') == std::string::npos) {
			if (settings.index.size()) {
				webpage = settings.root + request_info["Request-URI"] + settings.index;
			} else {
				webpage = settings.root + request_info["Request-URI"] + "/index.html";
			}
		} else {
			webpage = settings.root + request_info["Request-URI"] + "/index.html";
		}
		std::cout << "page: " << webpage << "\n";
		std::string resp;
		if (request_info["Request-URI"].size() > 1 && request_info["Request-URI"].find(".py") != std::string::npos) {
			if (request.headers["Method"] == "GET") {
				resp = get_cgi(request);
			} else {
				resp = get_cgi_post(request);
			}
		}
		else try {
			resp = get_reponse_from_page(webpage);
		} catch (...) {
			resp = not_found();
		}
		write(new_socket, resp.c_str(), resp.size());
		close(new_socket);
	}
}
