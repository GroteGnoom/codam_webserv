int	create_socket(void);
int	identify_socket(int socket, int port, sockaddr_in address);
int	listening_socket(int socket);
int	accept_socket(int socket, sockaddr_in address);