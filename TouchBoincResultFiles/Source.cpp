#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

int main()
{
	// touch every files with names from file with errors 

	std::ifstream error_file("errors");
	if (!error_file.is_open()) {
		std::cerr << "can't open file with name errors";
		return 1;
	}

	std::vector < std::string > files_names_vec;
	std::string str, prefix_str, launch_str;
	prefix_str = "couldn't copy file ";
	while (getline(error_file, str)) {
		if (str.find(prefix_str) != std::string::npos) {
			str = str.substr(prefix_str.size(), str.size() - prefix_str.size());
			if (std::find(files_names_vec.begin(), files_names_vec.end(), str) == files_names_vec.end()) {
				files_names_vec.push_back(str);
				launch_str = "touch " + str;
				system(launch_str.c_str());
				std::cout << "system command" << std::endl;
				std::cout << launch_str << std::endl;
			}
		}
	}
	error_file.close();

	return 0;
}