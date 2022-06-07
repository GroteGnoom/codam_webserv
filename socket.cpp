#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstring>

/*
* create a new socket
* returns socket id on success      */

int	create_socket(void) {
	int	server_fd;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0){
		perror("cannot create socket");
		return (0);
	}
	return (server_fd);
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
		perror("bind failed");
		return (0);
	}
	return (1);
}

/*
* make the socket prepared to accept connections (make it listen)
* returns 1 on success      */

int	listening_socket(int socket) {
	int	backlog;

	backlog = 10;
	if (listen(socket, backlog) < 0) {
		perror("listen failed");
		return (0);
	}
	return (1);
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
		perror("accept failed");
		return (0);
	}
	return (new_socket);
}
