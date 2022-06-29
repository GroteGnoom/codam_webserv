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
* bind a port to a socket
*/

int	set_socket_settings(int socket, int port, sockaddr_in &address) {
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
		perror("");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
* make the socket prepared to accept connections (make it listen)
*/

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
	return (new_socket);
}

int	method_allowed(std::string method, t_settings settings) {
	for (unsigned long i = 0; i < settings.unaccepted_methods.size(); i++) {
		if (method == settings.unaccepted_methods[i])
			return (0);
	}
	return (1);
}

t_server get_server(t_settings settings, int port, t_request request) {
	//std::cout << "host: " << request.headers["Host"] << "\n";
	std::string host = request.headers["Host"].substr(0, request.headers["Host"].find(':'));
	//std::cout << "host: " << host << "\n";
	for (size_t i = 0; i < settings.servers.size(); i++) {
		//std::cout << "name: " << settings.servers[i].name << "\n";
		if (port == settings.servers[i].listen_port && host == settings.servers[i].name) {
			//std::cout << "found server: " << i << "\n";
			return settings.servers[i];
		}
	}
	for (size_t i = 0; i < settings.servers.size(); i++) {
		if (port == settings.servers[i].listen_port) {
			//std::cout << "using default server: " << i << "\n";
			return settings.servers[i];
		}
	}
	std::cerr << "port not found in get_server, should never happen\n";
	exit(1);
}

std::string handle_request(t_request request, t_settings settings, int port) {
	std::string resp;
	std::string uri = request.headers["Request-URI"];
	std::string method = request.headers["Method"];
	t_server server = get_server(settings, port, request);

	if (!uri.compare(settings.redir_src))
		uri.replace(0, settings.redir_dst.size(), settings.redir_dst);

	std::string webpage;
	if (uri.find('.') == std::string::npos) {
		if (server.index.size()) {
			webpage = server.root + uri + server.index;
		} else {
			webpage = server.root + uri + "/index.html";
		}
	} else {
		webpage = server.root + uri;
	}
	// std::cout << "page: " << webpage << "\n";

	if (uri.size() > 1 && uri.find(".py") != std::string::npos) {
		if (method == "GET") {
			resp = get_cgi(request);
		}
	}
	else try {
		if (method == "GET" && method_allowed("GET", settings)) {
			try {
				resp = get_response_from_page(webpage);
			} catch (...) {
				resp = not_found();
			}
		}
		else if (method == "DELETE" && method_allowed("DELETE", settings)) {
			resp = get_delete(webpage);
		}
		else if (method == "POST" && method_allowed("POST", settings)) {
			resp = get_post(request, settings);
		} else if (!method_allowed(method, settings)){
			t_response response;
			response.body = "";
			response.code = 405;
			resp = response_to_string(response);
		} else {
			t_response response;
			response.body = "";
			response.code = 501;
			std::cout << method << "\n";
			resp = response_to_string(response);
		}
	} catch (...) {
		if (!server.index.size() && uri.find('.') == std::string::npos && server.locations[0].autoindex) { //TODO check location. autoindex is now a global setting :(
			resp = list_files(server.root + uri);
		}
		else resp = not_found();
	}
	// std::cout << resp << std::endl;
	return resp;
}

struct t_connection {
	int			port;
	int			fd;
	t_request	request;
	std::string	resp;
	bool operator <(const t_connection &other) const {
		return fd < other.fd;
	}
};

int	listen_to_new_socket(t_settings settings) {
	struct sockaddr_in					address;
	int									backlog = SOMAXCONN;	//how many requests can be backlogged
	std::set<t_connection>				connections;
	int nr_servers = settings.servers.size();
	std::cout << "nr servers: " << settings.servers.size() << "\n";
	std::set<int> ports;
	for (int i = 0; i < nr_servers; i++) {
		ports.insert(settings.servers[i].listen_port);
	}
	std::vector<int> port_vec(ports.begin(), ports.end());
	int nr_ports = ports.size();
	std::cout << "nr ports: " << nr_ports << "\n";

	struct pollfd pfd_init = {-1, POLLIN | POLLOUT, 0};
	std::vector<struct pollfd> pfd_ports(nr_ports, pfd_init);
	//struct pollfd pfd_conns[SOMAXCONN];

	for (int i = 0; i < nr_ports; i++) {
		pfd_ports[i].fd = socket(AF_INET, SOCK_STREAM, 0);
		if (pfd_ports[i].fd < 0) {
			perror("Failed to create socket: ");
			exit(EXIT_FAILURE);
		}
		if (set_socket_settings(pfd_ports[i].fd, port_vec[i], address) == EXIT_FAILURE)
			exit(EXIT_FAILURE);
		fcntl(pfd_ports[i].fd, F_SETFL, O_NONBLOCK);
		if (listening_socket(pfd_ports[i].fd, backlog) == EXIT_FAILURE)
			exit(EXIT_FAILURE);
	}
	while (1) {
		poll(&*pfd_ports.begin(), nr_ports, 0);
		if ((pfd_ports.begin()->revents)) {
			std::cout << "poll port: " << pfd_ports.begin()->revents << "\n";
		}
		for (int i = 0; i < nr_ports; i++) {
			if (!(pfd_ports[i].revents & POLLIN)) {
				continue;
			}
			pfd_ports[i].revents = 0;
			t_connection	new_conn;
			new_conn.fd = accept_socket(pfd_ports[i].fd, address);
			if (new_conn.fd >= 0) {
				fcntl(new_conn.fd, F_SETFL, O_NONBLOCK);
				if (new_conn.fd == EXIT_FAILURE)
					exit(EXIT_FAILURE);
				new_conn.request.state = RS_START;
				new_conn.request.written = 0;
				new_conn.port = port_vec[i];
				connections.insert(new_conn);
				// std::cout << "new connection\n";
			}
		}
		std::set<t_connection>::iterator iter = connections.begin();
		while ( iter != connections.end()) {
			//std::cout << "looping over connections!\n";
			const t_request *rpc = &(iter->request);
			t_request *rp = const_cast<t_request *>(rpc); //TODO why is this required
			const std::string *response = &(iter->resp);
			std::string *resp = const_cast<std::string *>(response); //TODO why is this required
			get_request_info(iter->fd, rp, resp);
			//std::cout << connections.size() << "\n";
			if (iter->request.state == RS_PROCESSING) {
				// std::cout << "request done reading!\n";
				// std::cout << rp->whole_request << "\n";
				// std::cout << rp->headers["Method"] << "\n";
				*resp = handle_request(iter->request, settings, iter->port); //only when a whole request is finished
				rp->state = RS_WRITING;
				std::cout << "done processing, now writing\n";
				// get_request_info(iter->fd, rp, resp);
				// std::cout << connections.size() << "\n";
			}
			if (iter->request.state == RS_CANCELLED || iter->request.state == RS_DONE) {
				close(iter->fd);
				std::set<t_connection>::iterator next = iter;
				next++;
				connections.erase(iter);
				iter = next;
				// std::cout << "connection removed\n";
				// std::cout << connections.size() << "\n";
				//exit(0);
			} else {
				iter++;
			}
		}
	}
}
