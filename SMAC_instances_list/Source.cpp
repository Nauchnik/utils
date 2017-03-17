#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>

void makeInstancesLists(char *problem_name_char_arr, unsigned vertices_number, unsigned instances_number);
void makeSimpleInstancesLists(char *folder_name_char_arr, char *problem_name_begin_char_arr, char *problem_name_end_char_arr, unsigned instances_number);

int main()
{
	makeSimpleInstancesLists("Bivium_MANY_AUX_VARS_34known", "Bivium_MANY_AUX_VARS_200bits_template_0_", ".cnf", 100);

	/*makeInstancesLists("BAR", 500,  100);
	makeInstancesLists("BAR", 1000, 100);
	makeInstancesLists("BAR", 2000, 10);
	makeInstancesLists("BAR", 4000, 10);
	makeInstancesLists("GNP", 500,  100);
	makeInstancesLists("GNP", 1000, 100);
	makeInstancesLists("GNP", 2000, 10);
	makeInstancesLists("GNP", 4000, 10);*/
	
	return 0;
}

void makeSimpleInstancesLists(char *folder_name_char_arr, char *problem_name_begin_char_arr, char *problem_name_end_char_arr, unsigned instances_number)
{
	std::string folder_name = folder_name_char_arr;
	std::string problem_name_begin = problem_name_begin_char_arr;
	std::string problem_name_end = problem_name_end_char_arr;
	std::stringstream sstream;
	std::string ofile_name;

	sstream << problem_name_begin << "all";
	ofile_name = sstream.str();
	sstream.str(""); sstream.clear();
	std::ofstream ofile(ofile_name.c_str());
	
	for (unsigned i = 0; i < instances_number; i++) {
		sstream << "instances/" << folder_name << "/" << problem_name_begin << i << problem_name_end << std::endl;
		ofile << sstream.rdbuf();
		sstream.str(""); sstream.clear();
	}
	ofile.close(); ofile.clear();
}

void makeInstancesLists(char *problem_name_char_arr, unsigned vertices_number, unsigned instances_number)
{
	std::string problem_name = problem_name_char_arr;
	std::stringstream sstream;
	std::ofstream ofile;
	std::string ofile_name;
	std::string cur_encoding;
	std::string add_instance_name;
	
	for (unsigned encoding_index = 0; encoding_index < 4; encoding_index++) {
		switch (encoding_index) {
		case 0: 
			cur_encoding = "CNF";
			add_instance_name = "";
			break;
		case 1: 
			cur_encoding = "GR_CNF"; 
			add_instance_name = "_gr";
			break;
		case 2: 
			cur_encoding = "RED_CNF";
			add_instance_name = "_re";
			break;
		case 3: 
			cur_encoding = "GR_RED_CNF"; 
			add_instance_name = "_gr_re";
			break;
		default: 
			cur_encoding = "CNF"; 
			add_instance_name = "";
			break;
		}
		sstream << problem_name << "_" << vertices_number << "_" << cur_encoding << "_all";
		ofile_name = sstream.str();
		sstream.str(""); sstream.clear();
		for (unsigned i = 0; i < instances_number; i++)
			sstream << "instances/" << problem_name << "/" << vertices_number << "/" 
				    << cur_encoding << "/" << problem_name << vertices_number 
			        << "_" << i << add_instance_name << ".cnf" << std::endl;
		ofile.open(ofile_name.c_str());
		ofile << sstream.rdbuf();
		sstream.str(""); sstream.clear();
		ofile.close(); ofile.clear();
	}
}

