#pragma once
#include <netinet/in.h>
#include "conf.hpp"

int	create_socket(void);
int	identify_socket(int socket, int port, sockaddr_in &address);
int	listening_socket(int socket, int backlog);
int	accept_socket(int socket, sockaddr_in address);
int	listen_to_new_socket(t_settings settings);
