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
	CRS_ACCESS_LOG,
	CRS_EXPECT_SC,
	CRS_EXPECT_BLOCK,
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

void process_token(conf_read_info *cri, std::string token) {
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
		} else if (token == "}") {
			cri->crs = CRS_GLOBAL;
		} else syntax_error(cri);
	} else if (cri->crs == CRS_SERVER) {
		if (token == "listen") {
			cri->crs = CRS_SERVER_LISTEN;
		} else if (token == "root") {
			cri->crs = CRS_SERVER_ROOT;
		} else if (token == "cgi") {
			cri->crs = CRS_SERVER_CGI;
		} else if (token == "index") {
			cri->crs = CRS_SERVER_INDEX;
		} else if (token == "}") {
			cri->crs = CRS_HTTP;
		} else syntax_error(cri);
	} else if (cri->crs == CRS_SERVER_LISTEN) {
		cri->settings.listen_port = atoi( token.c_str() );
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_ROOT) {
		if (token[0] != '/')
			cri->settings.root = cwd;
		cri->settings.root += token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_CGI) {
		cri->settings.cgi_path = token;
		cri->crs = CRS_EXPECT_SC;
		cri->next = CRS_SERVER;
	} else if (cri->crs == CRS_SERVER_INDEX) {
		cri->settings.index = token;
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

    for (std::vector<std::string>::iterator it = tokens.begin(); it < tokens.end(); it++) {
        process_token(&cri, *it);
    }
    return cri.settings;
};
