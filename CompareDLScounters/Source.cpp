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
	argv[2] = "dls9_count_data_Eduard";
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
	
	std::string file1_name = argv[1];
	std::string file2_name = argv[2];
	std::vector<wu> wu_vec_1 = readDataFromFile(file1_name);
	std::vector<wu> wu_vec_2 = readDataFromFile(file2_name);

	/*std::string str;
	std::ifstream ifile("Zerowu.txt");
	std::vector<int> check_id_vec;
	unsigned val;
	while (getline(ifile, str)) {
		std::istringstream(str.c_str()) >> val;
		check_id_vec.push_back(val);
	}
	ifile.close();
	unsigned match_count = 0, unmatch_count = 0;
	unsigned processed_wu_vec_2 = 0;
	std::vector<int>::iterator it;
	for ( auto &x : wu_vec_2 ) {
		if (x.processing_time < 0)
			continue;
		std::istringstream(x.first_cells_known_values) >> val;
		it = find(check_id_vec.begin(), check_id_vec.end(), val);
		if (it != check_id_vec.end()) {
			if (x.dls_number == 0)
				match_count++;
			else {
				unmatch_count++;
				std::cout << "unmatch wu_vec2 " << x.first_cells_known_values << std::endl;
			}
		}
		
		processed_wu_vec_2++;
		if (processed_wu_vec_2 % 1000 == 0)
			std::cout << "processed_wu_vec_2 " << processed_wu_vec_2 << std::endl;
	}
	std::cout << "match_count " << match_count << " from " << check_id_vec.size() << std::endl;
	std::cout << "unmatch_count " << unmatch_count << " from " << check_id_vec.size() << std::endl;*/
	
	bool is_calculated_results_correct = true;
	unsigned calculated_results_both_files = 0;
	double comparison_koef_sum = 0;
	double cur_comparison_koef;
	unsigned diff_results_count = 0;
	double calculated_sum1_time = 0, calculated_sum2_time = 0;
	unsigned long long total_dls_number_1 = 0, total_dls_number_2 = 0;
	for (unsigned i = 0; i < wu_vec_1.size(); i++) {
		if ( (wu_vec_1[i].dls_number >= 0) && (wu_vec_2[i].dls_number >= 0) ) { //&& 
			//(wu_vec_1[i].processing_time > 0) && (wu_vec_2[i].processing_time > 0)) {
			total_dls_number_1 += wu_vec_1[i].dls_number;
			total_dls_number_2 += wu_vec_2[i].dls_number;
			calculated_sum1_time += wu_vec_1[i].processing_time;
			calculated_sum2_time += wu_vec_2[i].processing_time;
			calculated_results_both_files++;
			cur_comparison_koef = (double)wu_vec_1[i].processing_time / (double)wu_vec_2[i].processing_time;
			if ( (cur_comparison_koef < 1) && (wu_vec_1[i].dls_number > 0) ) {
				//std::cout << "cur_comparison_koef < 1 " << cur_comparison_koef << std::endl;
				std::cout << "file1 : " << wu_vec_1[i].first_cells_known_values << " " << wu_vec_1[i].dls_number << " " << wu_vec_1[i].processing_time << std::endl;
				std::cout << "file2 : " << wu_vec_2[i].first_cells_known_values << " " << wu_vec_2[i].dls_number << " " << wu_vec_2[i].processing_time << std::endl;
				std::cout << "diff_results_count " << ++diff_results_count << std::endl;
				std::cout << std::endl;
			}
			if (wu_vec_1[i].dls_number != wu_vec_2[i].dls_number)
				is_calculated_results_correct = false;
			comparison_koef_sum += cur_comparison_koef;
			//std::cout << "cur_comparison_koef " << cur_comparison_koef << std::endl;
		}
	}
	
	double comparison_koef_average = comparison_koef_sum / calculated_results_both_files;

	std::cout.precision(10);
	std::cout << "calculated_results_both_files " << calculated_results_both_files << std::endl;
	std::cout << "is_calculated_results_correct " << is_calculated_results_correct << std::endl;
	std::cout << "comparison_koef_average "		  << comparison_koef_average	   << std::endl;
	std::cout << "calculated_sum1_time "	      << calculated_sum1_time          << std::endl;
	std::cout << "calculated_sum2_time "          << calculated_sum2_time          << std::endl;
	std::cout << "sum_koef (calculated_sum1_time / calculated_sum2_time) " << calculated_sum1_time / calculated_sum2_time << std::endl;
	std::cout << "total_dls_number_1 " << total_dls_number_1 << std::endl;
	std::cout << "total_dls_number_2 " << total_dls_number_2 << std::endl;
	
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
		cur_wu.processing_time = -1;
		sstream >> cur_wu.first_cells_known_values >> cur_wu.dls_number >> cur_wu.processing_time;
		sstream.str(""); sstream.clear();
		wu_vec.push_back(cur_wu);
	}
	cur_state_file.close();

	return wu_vec;
}