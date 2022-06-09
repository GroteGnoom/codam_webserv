#include <iostream>
#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "socket.hpp"

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

int	identify_socket(int socket, int port, sockaddr_in address) {
	memset((char *)&address, 0, sizeof(address));
	address.sin_family = AF_INET;					//adress family used in creating socket
	address.sin_addr.s_addr = htonl(INADDR_ANY);	//adress of this socket, this machines IP adress
	address.sin_port = htons(port);					//port number
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
		return (EXIT_FAILURE);
	}
	return (new_socket);
}

int	listen_to_new_socket(int port) {
	int	server_socket = create_socket();
	struct sockaddr_in	address;
	int	backlog = 1000;			//how many requests can be backlogged
	long	read_len;
	int		new_socket;

	if (server_socket == EXIT_FAILURE)
		exit(EXIT_FAILURE);
	if (identify_socket(server_socket, port, address) == EXIT_FAILURE)
		exit(EXIT_FAILURE);
	if (listening_socket(server_socket, backlog) == EXIT_FAILURE)
		exit(EXIT_FAILURE);
	while (1) {
		new_socket = accept_socket(server_socket, address);
		if (new_socket == EXIT_FAILURE)
			exit(EXIT_FAILURE);
		char	buffer[30000] = {0};
		read_len = read(new_socket, buffer, 30000);
		std::cout << buffer << std::endl;
	}
}
