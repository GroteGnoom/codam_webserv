#include <string>
#include <dirent.h>
#include <iostream>
#include <vector>
#include "list_files.hpp"
#include "response.hpp"

//returns a file listing of a directory in HTML, where the file names should be links.
//example: https://stackoverflow.com/a/3793748

std::string list_files(std::string path, std::string uri) {
	std::vector<std::string> filenames = all_files_in_dir(path);
	std::string result;

	result += "<head><style> div { background-color: gold; width: 300px; border: 15px solid purple; padding: 50px; margin: 20px; } </style> </head> \n";
	result += "<div>";
	result += "<h1> Index of " + uri + "</h1><br>\n";
	for (size_t i = 0; i < filenames.size(); i++) {
		result += "<a href=" + uri + "/" + filenames[i] + ">" + filenames[i] + "</a><br>\n";
	}
	result += "</div>";
	std::cout << "returned files\n";
	return get_response(result, 1);
}

std::vector<std::string> all_files_in_dir(std::string path) {
	DIR *dir;
	struct dirent *ent;
	dir = opendir (path.c_str()) ;
	std::vector<std::string> result;
	
	if (dir != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
				result.push_back(ent->d_name);
		}
		closedir (dir);
	} else {
		std::cout << "could not find dir:" << path << "\n";
		throw std::exception();
	}
	return result;
}
