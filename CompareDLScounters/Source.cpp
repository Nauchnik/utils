#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

struct wu
{
	std::string first_cells_known_values;
	long long dls_number;
	double processing_time;
};

std::vector<wu> readDataFromFile( std::string filename );

int main( int argc, char* argv[])
{
#ifdef _DEBUG
	argc = 3;
	argv[1] = "dls9_count_data_Eduard";
	argv[2] = "dls9_count_data_Stepa";
#endif

	std::stringstream sstream1;
	sstream1 << "123 555";
	std::string str1, str2, str3;
	double dval = -1;
	sstream1 >> str1 >> str2 >> dval;

	if (argc != 3) {
		std::cerr << "Usage : program file_data_eduard file_data_stepa";
		return 1;
	}
	
	std::string eduard_file_name = argv[1];
	std::string stepa_file_name = argv[2];
	std::vector<wu> wu_vec_eduard = readDataFromFile(eduard_file_name);
	std::vector<wu> wu_vec_stepa = readDataFromFile(stepa_file_name);

	bool is_calculated_results_correct = true;
	unsigned calculated_results_both_files = 0;
	double comparison_koef_sum = 0;
	double cur_comparison_koef;
	for (unsigned i = 0; i < wu_vec_eduard.size(); i++) {
		if ((wu_vec_eduard[i].dls_number >= 0) && (wu_vec_stepa[i].dls_number >= 0)) {
			calculated_results_both_files++;
			cur_comparison_koef = (double)wu_vec_eduard[i].processing_time / (double)wu_vec_stepa[i].processing_time;
			if ( (cur_comparison_koef < 1) && (wu_vec_eduard[i].dls_number > 0) ) {
				std::cout << "cur_comparison_koef < 1 " << cur_comparison_koef << std::endl;
				std::cout << "eduard : " << wu_vec_eduard[i].first_cells_known_values << " " << wu_vec_eduard[i].dls_number << " " << wu_vec_eduard[i].processing_time << std::endl;
				std::cout << "stepa : " << wu_vec_stepa[i].first_cells_known_values << " " << wu_vec_stepa[i].dls_number << " " << wu_vec_stepa[i].processing_time << std::endl;
				std::cout << std::endl;
			}
			if (wu_vec_eduard[i].dls_number != wu_vec_stepa[i].dls_number)
				is_calculated_results_correct = false;
			comparison_koef_sum += cur_comparison_koef;
		}
	}
	
	double comparison_koef_average = comparison_koef_sum / calculated_results_both_files;

	std::cout << "calculated_results_both_files " << calculated_results_both_files << std::endl;
	std::cout << "is_calculated_results_correct " << is_calculated_results_correct << std::endl;
	std::cout << "comparison_koef_average "		  << comparison_koef_average	   << std::endl;
	
	return 0;
}

std::vector<wu> readDataFromFile(std::string cur_state_file_name)
{
	std::vector<wu> wu_vec;
	std::string str;
	wu cur_wu;
	std::stringstream sstream;
	std::ifstream cur_state_file(cur_state_file_name.c_str());

	// read current states of WUs from the input file
	wu_vec.reserve(1256000);
	int first_non_started_wu_index = -1;
	while (getline(cur_state_file, str)) {
		sstream << str;
		sstream >> cur_wu.first_cells_known_values >> cur_wu.dls_number >> cur_wu.processing_time;
		sstream.str(""); sstream.clear();
		wu_vec.push_back(cur_wu);
	}
	cur_state_file.close();

	return wu_vec;
}