#include "conf.hpp"
#include <sstream>

enum conf_read_state{
    CRS_GLOBAL,
    CRS_HTTP,
    CRS_SERVER,
    CRS_EVENTS
};

/*
enum t_token_type {
    TT_STRING,
    TT_OPEN_BRACE,
    TT_CLOSE_BRACE,
    TT_SEMICOLON,
};

struct t_token {
    t_token_type type;
    std::string str;
}
*/

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

void directive(conf_read_state *crs, std::string directive) {
    if (*crs == CRS_GLOBAL) {
        if (directive == "error_log") {
            //TODO
        } else if (directive == "http") {
            //TODO
        } else if (directive == "events") {
            //TODO
        } else {
            //logger.log(EMERG) << unknown directive \"" << directive << "\" in << conf_file << ":" << line_nr << "\n";
            //TODO what should the interface of the logger be?

        }
    }
}

/* should just split stuff up into strings
 * so "bla{}bloe;ble\nblie#comment" should become
 * {"bla", "{", "}", "bloe", ";", "ble", "blie"} */
std::vector<std::string> tokenize(std::string conf) {
   std::string::size_type pos, lastPos = 0;
   std::vector<std::string> tokens;

   while(lastPos < conf.length() + 1)
   {
      pos = conf.find_first_of(" \n;{}", lastPos);
      if(pos == std::string::npos)
      {
         pos = conf.length();
      }

      if(pos == lastPos && (conf[pos] == ' ' || conf[pos] == '\n')) {
          continue;
      }
      std::string token(&conf[lastPos], pos-lastPos);
      tokens.push_back(token);

      lastPos = pos + 1;
   }
   return tokens;
}

t_settings read_conf(char *conf_file) {
    t_settings settings;
    conf_read_state crs = CRS_GLOBAL;
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

    for (std::vector<std::string>::iterator it = tokens.begin(); it < tokens.end(); it++) {
        directive(&crs, *it); //TODO split up in blocks? make an ast?
    }
    return settings;
};
