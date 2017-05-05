#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <vector>

int main(int argc, char** argv)
{
	std::vector<std::string> files_names;
	files_names.push_back("*out_total_eva500a_geffe96_cnfs_2rail");
	files_names.push_back("*out_total_mscg15a_geffe96_cnfs_2rail");
	files_names.push_back("*out_total_mscg15b_geffe96_cnfs_2rail");
	files_names.push_back("*out_total_open-wbo16_geffe96_cnfs_2rail");
	files_names.push_back("*out_total_wpm3-2015-co_geffe96_cnfs_2rail");

	
	std::vector<std::vector<double>> values_vec_vec;
	std::vector<std::string> cnfs_names;
	std::string str, word1;
	std::stringstream sstream;
	for (unsigned i = 0; i < files_names.size(); i++) {
		bool isGotFiles = true;
		if (cnfs_names.size() == 0)
			isGotFiles = false;
		std::ifstream ifile(files_names[i]);
		if (ifile.is_open()) {
			std::vector<double> cur_values_vec;
			while (getline(ifile,str)) {
				sstream << str;
				double dval;
				sstream >> word1 >> dval;
				sstream.str("");
				sstream.clear();
				if (!isGotFiles)
					cnfs_names.push_back(word1);
				cur_values_vec.push_back(dval);
			}
			values_vec_vec.push_back(cur_values_vec);
		}
	}

	std::string head_str = "Instance eva500a mscg15a mscg15b open-wbo16 wpm3-2015-co";
	std::ofstream ofile("geffe_csv");
	ofile << head_str << std::endl;
	for (unsigned i=0; i < cnfs_names.size(); i++) {
		ofile << cnfs_names[i] << " ";
		for (unsigned j = 0; j < values_vec_vec.size(); j++)
			ofile << values_vec_vec[j][i];
		ofile << std::endl;
	}
	ofile.close();

	return 0;
}