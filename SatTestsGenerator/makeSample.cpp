#include "makeSample.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
boost::random::mt19937 gen(static_cast<unsigned int>(std::time(0)));
using namespace Addit_func;

makeSample::makeSample() :
	launchType(RANDOM_UNSAT_SAMPLE),
	tests_count(0)
{
	gen.seed(static_cast<unsigned>(std::time(0)));
}

void makeSample::readInput(int argc, char **argv)
{
	cnf_file_name = argv[1];
	std::cout << "cnf_file_name " << cnf_file_name << std::endl;
	cut_cnf_file_name = cnf_file_name;
	while ((cut_cnf_file_name[0] == '.') || (cut_cnf_file_name[0] == '/'))
		cut_cnf_file_name = cut_cnf_file_name.substr(1, cut_cnf_file_name.length() - 1);
	std::cout << "cut_cnf_file_name " << cut_cnf_file_name << std::endl;

	decomp_set_file_name = argv[2];
	std::cout << "decomp_set_file_name " << decomp_set_file_name << std::endl;

	std::string third_param_str = argv[3];
	
	if ( third_param_str == "-input_output_assumptions" ) {
		launchType = INPUT_OUTPUT_ASSUMPTIONS_SAMPLE;
		std::cout << "launchType " << launchType << std::endl;
		input_output_folder_name = argv[4];
		std::cout << "input_output_folder_name " << input_output_folder_name << std::endl;
		return;
	}
	
	tests_count = atoi(third_param_str.c_str());
	std::cout << "tests_count " << tests_count << std::endl;

	std::string str;
	if (argc > 4) {
		str = argv[4];
		if (str.find("sat") != std::string::npos)
			launchType = RANDOM_SAT_SAMPLE;
		else if (str.find("assumptions") != std::string::npos)
			launchType = ASSUMPTIONS_SAMPLE;
	}
	if ( (argc > 5) && ( launchType == ASSUMPTIONS_SAMPLE ) )
		assumptions_file_name = argv[5];

	std::cout << "launchType " << launchType << std::endl;
}

void makeSample::init()
{
	cnf_file.open(cnf_file_name.c_str());
	if (!cnf_file.is_open()) {
		std::cerr << "Error. !cnf_file.is_open()" << std::endl;
		exit(1);
	}
	
	/*if (tests_count <= 0) {
		std::cerr << "Error. tests_count <= 0" << std::endl;
		exit(1);
	}*/

	test_cnf_files.resize(tests_count);

	unsigned found = cut_cnf_file_name.find(".");
	cnf_name_common_part = (found != std::string::npos) ? cut_cnf_file_name.substr(0, found) : cut_cnf_file_name;
	std::cout << "cnf_name_common_part " << cnf_name_common_part << std::endl;

	unsigned comment_str_count = 0, main_str_count = 0;
	std::string str;
	while (getline(cnf_file, str)) {
		str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
		if (str[0] == 'c') {
			comment_cnf_sstream << str << std::endl;
			comment_str_count++;
		}
		else if (str[0] != 'p') {
			if (main_str_count)
				main_cnf_sstream << std::endl;
			main_cnf_sstream << str;
			main_str_count++;
		}
	}
	std::cout << "main_str_count " << main_str_count << std::endl;

	bool isDecompSet = false;
	if (decomp_set_file_name != "no") {
		decomp_set_file.open(decomp_set_file_name.c_str());
		isDecompSet = true;
		if (!decomp_set_file.is_open()) {
			std::cerr << "Error. !cnf_file.is_open()" << std::endl;
			exit(1);
		}
	}
	int from, to;
	if (isDecompSet) {
		unsigned val = 0;
		std::cout << "reading decomp_set" << std::endl;
		getline(decomp_set_file, str);
		decomp_set_file.close();
		decomp_set_file.clear();
		val = str.find('-');
		std::cout << "val " << val << std::endl;
		if ( val < 1000000 ) {
			std::cout << "reading interval" << std::endl;
			std::string val_from_str, val_to_str;
			val_from_str = str.substr( 0, val );
			val_to_str = str.substr(val + 1, str.size() - val_from_str.size());
			std::istringstream(val_from_str) >> from;
			std::istringstream(val_to_str) >> to;
			for (int i = from; i <= to; i++ )
				decomp_set.push_back(i);
		}
		else {
			std::cout << "reading separate variables" << std::endl;
			decomp_set_file.open(decomp_set_file_name.c_str());
			while (decomp_set_file >> val) {
				std::cout << val << " ";
				decomp_set.push_back(val);
			}
			decomp_set_file.close();
		}
	}
	std::cout << std::endl;
	std::cout << "decomp_set.size() " << decomp_set.size() << std::endl;

	mpi_b.isMakeSatSampleAnyWay = true;
	mpi_b.input_cnf_name = cut_cnf_file_name;
	mpi_b.ReadIntCNF();
	std::cout << "mpi_b.var_count " << mpi_b.var_count << std::endl;
	std::cout << "mpi_b.clause_count " << mpi_b.clause_count << std::endl;

	std::vector< std::vector<bool> > plain_text_vec_vec;
	
	if ( launchType == RANDOM_SAT_SAMPLE ) {
		mpi_b.cnf_in_set_count = tests_count;
		mpi_b.MakeSatSample(state_vec_vec, stream_vec_vec, plain_text_vec_vec, 0);
	}
	
	for (unsigned i = 0; i < mpi_b.output_len; i++)
		output_variables.push_back(mpi_b.var_count - mpi_b.output_len + i + 1);
	for (unsigned i = 0; i < mpi_b.core_len; i++)
		input_variables.push_back(i + 1);

	std::vector<unsigned>::iterator it;
	for (unsigned i = 0; i < decomp_set.size(); i++) {
		it = find(input_variables.begin(), input_variables.end(), decomp_set[i]);
		if (it != input_variables.end())
			decomp_set_indexes.push_back(it - input_variables.begin());
	}

	unsigned new_clause_count;
	if ((launchType == RANDOM_SAT_SAMPLE) || (launchType == INPUT_OUTPUT_ASSUMPTIONS_SAMPLE))
		new_clause_count = mpi_b.clause_count + decomp_set.size() + output_variables.size();
	else
		new_clause_count = mpi_b.clause_count + decomp_set.size();

	head_cnf_sstream << "p cnf " << mpi_b.var_count << " " << new_clause_count << std::endl;

	cnf_file.close();
}

void makeSample::makeRandomUnsatSample()
{
	std::stringstream current_name_sstream;
	for (unsigned i = 0; i < test_cnf_files.size(); i++) {
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new std::ofstream(current_name_sstream.str().c_str());
		(*test_cnf_files[i]) << comment_cnf_sstream.str(); // write comments of cnf file
		(*test_cnf_files[i]) << head_cnf_sstream.str(); // write head of cnf file
														// write oneliteral clauses
		for (unsigned j = 0; j < decomp_set.size(); j++) {
			if (bool_rand(gen))
				(*test_cnf_files[i]) << "-";
			(*test_cnf_files[i]) << decomp_set[j] << " 0" << std::endl;
		}

		(*test_cnf_files[i]) << main_cnf_sstream.str(); // write clauses of main cnf
		(*test_cnf_files[i]).close();
		delete test_cnf_files[i];
		current_name_sstream.clear(); current_name_sstream.str("");
	}
	std::cout << tests_count << " tests were created" << std::endl;
}

void makeSample::makeRandomSatSample()
{
	std::cout << "Start makeRandomSatSample()" << std::endl;
	unsigned cur_var_ind;
	std::stringstream oneliteral_sstream, current_name_sstream;
	for (unsigned i = 0; i < tests_count; i++) {
		for (std::vector<unsigned>::iterator it = decomp_set.begin(); it != decomp_set.end(); it++) {
			cur_var_ind = (*it) - 1;
			if (!(state_vec_vec[i][cur_var_ind]))
				oneliteral_sstream << "-";
			oneliteral_sstream << cur_var_ind + 1 << " 0" << std::endl;
		}
		unsigned cur_stream_index = 0;
		for (std::vector<bool>::iterator it = stream_vec_vec[i].begin(); it != stream_vec_vec[i].end(); it++) {
			cur_var_ind = (mpi_b.var_count - mpi_b.output_len) + cur_stream_index;
			if (!(*it))
				oneliteral_sstream << "-";
			oneliteral_sstream << cur_var_ind + 1 << " 0" << std::endl;
			cur_stream_index++;
		}
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new std::ofstream(current_name_sstream.str().c_str());
		(*test_cnf_files[i]) << comment_cnf_sstream.str(); // write comments of cnf file
		(*test_cnf_files[i]) << head_cnf_sstream.str();    // write head of cnf file
		(*test_cnf_files[i]) << oneliteral_sstream.str();  // write oneliteral clauses
		(*test_cnf_files[i]) << main_cnf_sstream.str();    // write clauses of main cnf
		(*test_cnf_files[i]).close();
		delete test_cnf_files[i];
		current_name_sstream.clear(); current_name_sstream.str("");
		oneliteral_sstream.str(""); oneliteral_sstream.clear();
	}
}

void makeSample::makeSampleFromAssumptions()
{
	std::ifstream assumptions_file(assumptions_file_name);
	if (!assumptions_file.is_open()) {
		std::cerr << "assumptions_file " << assumptions_file_name << " open fails " << std::endl;
		exit(1);
	}
	std::string str;
	std::vector<std::string> var_values_vec;
	std::stringstream sstream;
	std::string tmp_str;
	bool isMatchString;
	while (getline(assumptions_file, str)) { // read values of variables
		sstream << str;
		while (sstream >> tmp_str) {
			if (tmp_str.size() < 5)
				continue;
			isMatchString = true;
			for (auto &x : tmp_str)
				if ((x != '1') && (x != '0')) {
					isMatchString = false;
					break;
				}
			if (isMatchString)
				break;
		}
		var_values_vec.push_back(tmp_str);
		sstream.clear(); sstream.str("");
	}

	assumptions_file.close();
	tests_count = var_values_vec.size();
	test_cnf_files.resize(tests_count);
	std::stringstream current_name_sstream;
	for (unsigned i = 0; i < test_cnf_files.size(); i++) {
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new std::ofstream(current_name_sstream.str().c_str());
		(*test_cnf_files[i]) << comment_cnf_sstream.str(); // write comments of cnf file
		(*test_cnf_files[i]) << head_cnf_sstream.str(); // write head of cnf file
														// write oneliteral clauses
		for (unsigned j = 0; j < decomp_set.size(); j++) // write corresponding oneliteral clauses
			(*test_cnf_files[i]) << (var_values_vec[i][j] == '1' ? "" : "-") << decomp_set[j] << " 0" << std::endl;
		(*test_cnf_files[i]) << main_cnf_sstream.str(); // write clauses of main cnf
		(*test_cnf_files[i]).close();
		delete test_cnf_files[i];
		current_name_sstream.clear(); current_name_sstream.str("");
	}

	std::cout << tests_count << " tests were created" << std::endl;
}

void makeSample::makeSampleFromInputOutputAssumptions()
{
	std::vector<std::string> input_output_files_names = std::vector<std::string>();

	Addit_func::getdir(input_output_folder_name, input_output_files_names);
	
	std::cout << "input_output_files_names.size() " << input_output_files_names.size() << std::endl;
	
	std::string cur_file_name;
	std::fstream cur_file;
	std::string input_variables_values_str, output_variables_values_str, start_str;
	std::stringstream oneliteral_sstream;
	std::string cnf_folder_name = input_output_folder_name + "_cnfs";

	for ( auto &x : input_output_files_names ) {
		cur_file_name = "./" + input_output_folder_name + "/" + x;
		if (cur_file_name.find(".cnf") != std::string::npos)
			continue;
		cur_file.open(cur_file_name.c_str(), std::ios_base::in);
		if (!cur_file.is_open()) {
			std::cerr << "couldn't open " << cur_file_name << std::endl;
			exit(1);
		}
		getline(cur_file, input_variables_values_str);
		start_str = "Input: ";
		input_variables_values_str = input_variables_values_str.substr(start_str.size(), input_variables_values_str.size() - start_str.size());
		getline(cur_file, output_variables_values_str);
		start_str = "Output: ";
		output_variables_values_str = output_variables_values_str.substr(start_str.size(), output_variables_values_str.size() - start_str.size());
		if ( (mpi_b.output_len>0) && (mpi_b.output_len < output_variables_values_str.size()) )
			output_variables_values_str.resize(mpi_b.output_len);
		cur_file.clear();
		cur_file.close();
		// make CNF file with additional clauses
		cur_file_name = "./" + cnf_folder_name + "/" + x + ".cnf";
		cur_file.open(cur_file_name.c_str(), std::ios_base::out);
		if (!cur_file.is_open()) {
			std::cerr << "couldn't open " << cur_file_name << std::endl;
			exit(1);
		}
		for (unsigned i = 0; i < decomp_set_indexes.size(); i++) {
			if (input_variables_values_str[decomp_set_indexes[i]] == '0')
				oneliteral_sstream << "-";
			oneliteral_sstream << decomp_set[i] << " 0" << std::endl;
		}
		//for (cur_stream_index = 0; cur_stream_index < mpi_b.keystream_len; cur_stream_index++) {
		for (unsigned i = 0; i < output_variables.size(); i++) {
			if (output_variables_values_str[i] == '0')
				oneliteral_sstream << "-";
			oneliteral_sstream << output_variables[i] << " 0" << std::endl;
		}
		cur_file << comment_cnf_sstream.str(); // write comments of cnf file
		cur_file << head_cnf_sstream.str();    // write head of cnf file
		cur_file << oneliteral_sstream.str();  // write oneliteral clauses
		cur_file << main_cnf_sstream.str();    // write clauses of main cnf
		cur_file.clear();
		cur_file.close();
		oneliteral_sstream.str(""); oneliteral_sstream.clear();
	}
}