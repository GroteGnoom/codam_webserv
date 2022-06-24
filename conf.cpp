#include "conf.hpp"
#include <sstream>
#include <unistd.h>

enum conf_read_state{
    CRS_GLOBAL,
    CRS_HTTP,
    CRS_SERVER,
    CRS_EVENTS,
    CRS_ERROR_LOG,
    CRS_ERROR_LOG_LEVEL,
    CRS_SERVER_LISTEN,
    CRS_SERVER_ROOT,
	CRS_SERVER_CGI,
	CRS_SERVER_INDEX,
	CRS_SERVER_SIZE,
	CRS_LOCATION_STRING,
	CRS_LOCATION,
	CRS_SERVER_NAME,
	CRS_ACCESS_LOG,
	CRS_EXPECT_SC,
	CRS_EXPECT_BLOCK,
	CRS_AUTOINDEX,
	CRS_ACCEPT_METHOD,
	CRS_SERVER_REDIRECT_SRC,
	CRS_SERVER_REDIRECT_DST,
};

struct conf_read_info {
	conf_read_state crs;
	conf_read_state next;
	t_settings settings;
};

std::vector<std::string> split_on_whitespace(std::string str) {
    std::string buf;
    std::stringstream ss(str);

    std::vector<std::string> tokens;
    while (ss >> buf) {
        if (!buf.empty())
            tokens.push_back(buf);   
    }
    return tokens;
}

void syntax_error(conf_read_info *cri) {
	(void) cri;
	std::cerr << "state: " << cri->crs << "\n";
	std::cerr << "next: " << cri->next << "\n";
	throw std::exception();
}

void process_token(conf_read_info *cri, std::string token, int *server_index) {
	char		buf[256];
	std::string	cwd;

	cwd.append(getcwd(buf, 256));
	cwd += '/';
    if (cri->crs == CRS_EXPECT_BLOCK) {
		if (token == "{") {
			cri->crs = cri->next;
		} else {
			syntax_error(cri);
		}
	} else if (cri->crs == CRS_EXPECT_SC) {
		if (token == ";") {
			cri->crs = cri->next;
		} else {
			syntax_error(cri);
		}
	} else if (cri->crs == CRS_GLOBAL) {
        if (token == "error_log") {
            cri->crs = CRS_ERROR_LOG;
        } else if (token == "http") {
            cri->crs = CRS_EXPECT_BLOCK;
            cri->next = CRS_HTTP;
        } else if (token == "events") {
            cri->crs = CRS_EVENTS;
        } else {
            //logger.log(EMERG) << unknown token \"" << token << "\" in << conf_file << ":" << line_nr << "\n";
            //TODO what should the interface of the logger be?
			syntax_error(cri);
        }
    } else if (cri->crs == CRS_HTTP) {
		if (token == "access_log") {
			cri->crs = CRS_ACCESS_LOG;
		} else if (token == "server") {
			cri->crs = CRS_EXPECT_BLOCK;
			cri->next = CRS_SERVER;
			(*server_index)++;
			cri->settings.servers.resize(cri->settings.servers.size() + 1);
		} else if (token == "}") {
			cri->crs = CRS_GLOBAL;
		} else syntax_error(cri);

	} else if (cri->crs == CRS_SERVER) {
		if (token == "listen") {
			cri->crs = CRS_SERVER_LISTEN;
		} else if (token == "root") {
			cri->crs = CRS_SERVER_ROOT;
		} else if (token == "redirect") {
			cri->crs = CRS_SERVER_REDIRECT_SRC;
		} else if (token == "location") {
			cri->crs = CRS_LOCATION_STRING;
			cri->settings.servers[*server_index].locations.resize(cri->settings.servers[*server_index].locations.size() + 1);
		} else if (token == "server_name") {
			cri->crs = CRS_SERVER_NAME;
		} else if (token == "client_max_body_size") {
			cri->crs = CRS_SERVER_SIZE;
		} else if (token == "cgi") {
			cri->crs = CRS_SERVER_CGI;
		} else if (token == "index") {
			cri->crs = CRS_SERVER_INDEX;
		} else if (token == "}") {
			cri->crs = CRS_HTTP;
		} else syntax_error(cri);

	} else if (cri->crs == CRS_SERVER_LISTEN) {
		cri->settings.servers[*server_index].listen_port = atoi( token.c_str() );
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_ROOT) {
		if (token[0] != '/')
			cri->settings.servers[*server_index].root = cwd;
		cri->settings.servers[*server_index].root += token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_CGI) {
		cri->settings.servers[*server_index].cgi_path = token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_NAME) {
		cri->settings.servers[*server_index].name = token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_INDEX) {
		cri->settings.servers[*server_index].index = token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_REDIRECT_SRC) {
		cri->settings.redir_src = token;
		cri->crs = CRS_SERVER_REDIRECT_DST;
	} else if (cri->crs == CRS_SERVER_REDIRECT_DST) {
		cri->settings.redir_dst = token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_SIZE) {
		if (token.back() != 'B' && token.back() != 'K' && token.back() != 'M' && token.back() != 'G') {
			std::cerr << "unexpected token in location: " << token << "\n";
			throw std::exception();
		}
		if (token.substr(0, (token.size() - 1)).find_first_not_of("0123456789") != std::string::npos) {
			std::cerr << "unexpected token in location: " << token << "\n";
			throw std::exception();
		}
		cri->settings.size_string = std::stoi(token.substr(0, (token.size() - 1)));
		if (token.back() == 'K')
			cri->settings.size_string *= 1000;
		else if (token.back() == 'M')
			cri->settings.size_string *= 1000000;
		else if (token.back() == 'G')
			cri->settings.size_string *= 1000000000;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_ACCESS_LOG) {
		cri->settings.access_log = cwd;
		cri->settings.access_log += token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_HTTP;
	} else if (cri->crs == CRS_ERROR_LOG) {
		cri->settings.error_log = cwd;
		cri->settings.error_log += token;
		cri->crs = CRS_ERROR_LOG_LEVEL;
	} else if (cri->crs == CRS_ERROR_LOG_LEVEL) {
		cri->settings.error_level = token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_GLOBAL;
	} else if (cri->crs == CRS_LOCATION_STRING) {
		cri->settings.servers[*server_index].locations[0].location = token;
		cri->crs = CRS_EXPECT_BLOCK;
		cri->next = CRS_LOCATION;
	} else if (cri->crs == CRS_LOCATION) {
		if (token == "autoindex") {
			cri->crs = CRS_AUTOINDEX;
		} else if (token == "off"){
			cri->crs = CRS_ACCEPT_METHOD;
		} else if (token == "}") {
			cri->crs = CRS_SERVER;
		} else {
			std::cerr << "unexpected token in location: " << token << "\n";
			throw std::exception();
		}
	} else if (cri->crs == CRS_AUTOINDEX) {
		if (token == "on") {
			cri->settings.servers[*server_index].locations[0].autoindex = true;
			cri->crs = CRS_EXPECT_SC;
			cri->next = CRS_LOCATION;
		} else if (token == "off") {
			cri->settings.servers[*server_index].locations[0].autoindex = false;
			cri->crs = CRS_EXPECT_SC;
			cri->next = CRS_LOCATION;
		}
	} else if (cri->crs == CRS_ACCEPT_METHOD) {
		cri->settings.unaccepted_methods.push_back(token);
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_LOCATION;
	}
}

std::ostream &operator <<(std::ostream &out, const std::vector<std::string> &v) {
	out << "{";
	std::vector<std::string>::size_type i = 0;
	for (; i < v.size() -1; i++) {
		out << '"' << v[i] << '"';
		out << ", ";
	}
	if (v.size())
		out << i;
	out << "}";
	return out;
}

/* should just split stuff up into strings
 * so "bla{}bloe;ble\nblie#comment" should become
 * {"bla", "{", "}", "bloe", ";", "ble", "blie"} */
std::vector<std::string> tokenize(std::string conf) {
	std::string::size_type end;
	std::vector<std::string> tokens;

	for(std::string::size_type i = 0; i < conf.length();)
	{
		/*special characters*/
		if (conf[i] == '{' || conf[i] == ';' || conf[i] == '}') {
			std::string token(&conf[i], 1);
			tokens.push_back(token);
			i++;
			continue;
		/*whitespace*/
		} else if (conf[i] == ' ' || conf[i] == '\n' || conf[i] == '\t') {
			i++;
			continue;
		/*comments*/
		} else if (conf[i] == '#') {
			end = conf.find_first_of("\n", i);
			i = end;
		/*other strings*/
		} else {
			end = conf.find_first_of(" \t\n;{}", i);
			if(end == std::string::npos)
			{
				end = conf.length();
			}
			std::string token(&conf[i], end-i);
			tokens.push_back(token);
			i = end;
		}
	}
   return tokens;
}

t_settings read_conf(char *conf_file) {
	//std::cout << "read_conf\n";
	conf_read_info cri = {};
    cri.crs = CRS_GLOBAL;
    std::ifstream input_stream(conf_file);

    // check stream status
    if (!input_stream) {
        std::cerr << "Can't open config file " << conf_file << "\n";
        exit(1);
    }
    std::ostringstream sstr;
    sstr << input_stream.rdbuf();
    std::vector<std::string> tokens = tokenize(sstr.str());
    input_stream.close();

	//std::cout << tokens << "\n";

	int	server_index = -1;

    for (std::vector<std::string>::iterator it = tokens.begin(); it < tokens.end(); it++) {
        process_token(&cri, *it, &server_index);
    }
	std::cout << cri.settings.servers[0].name << std::endl;
	std::cout << cri.settings.servers[1].name << std::endl;
	std::cout << cri.settings.servers.size() << std::endl;
	// std::cout << "redir: " << cri.settings.redir_src << "\n";
    return cri.settings;
};
