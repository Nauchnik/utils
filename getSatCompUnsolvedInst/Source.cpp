#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

struct instance_stats
{
	std::string name;
	unsigned solved = 0;
};

int main()
{
	std::string filename = "main-app.csv";
	std::ifstream ifile(filename);
	std::string str;
	std::vector<std::string> unsolved_inst_vec;
	bool isUnsolved = true;
	std::string current_inst = "", prev_inst = "";
	getline(ifile, str); // skip head string

	std::vector<instance_stats> inst_stats_vec;
	
	while (getline(ifile,str)) {
		std::replace(str.begin(), str.end(), ',', ' ');
		std::stringstream sstream;
		std::vector<std::string> word_vec;
		std::string word;
		sstream << str;
		while ( sstream >> word )
			word_vec.push_back(word);
		bool isInstAdded = false;
		if (word_vec[0] == "16pipe_16_ooo")
			word = str;
		for (auto &x : inst_stats_vec) {
			if (x.name == word_vec[0]) {
				isInstAdded = true;
				if (word_vec[5].find("complete") != std::string::npos)
					x.solved++;
			}
		}
		if (!isInstAdded) {
			instance_stats cur_i_s;
			cur_i_s.name = word_vec[0];
			if (word_vec[5].find("timeout") == std::string::npos)
				cur_i_s.solved = 1;
			else
				cur_i_s.solved = 0;
			inst_stats_vec.push_back(cur_i_s);
		}
	}
	ifile.close();
	std::ofstream ofile("out");
	for (auto &x : inst_stats_vec)
		if (x.solved == 0)
			ofile << x.name << std::endl;
	ofile.close();
	return 0;
}