#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "makeSample.h"
boost::random::mt19937 gen(static_cast<unsigned int>(std::time(0)));
using namespace Addit_func;

makeSample::makeSample() :
	isSatSample(false)
{
	gen.seed(static_cast<unsigned>(std::time(0)));
}

bool makeSample::ifSatSample() { return isSatSample; }

void makeSample::readInput(int argc, char **argv)
{
	if (argc < 4) {
		std::cerr << "Usage: cnf_file decomp_set_file tests_count [-sat]";
		exit(1);
	}

	cnf_file_name = argv[1];
	std::cout << "cnf_file_name " << cnf_file_name << std::endl;
	cut_cnf_file_name = cnf_file_name;
	while ((cut_cnf_file_name[0] == '.') || (cut_cnf_file_name[0] == '/'))
		cut_cnf_file_name = cut_cnf_file_name.substr(1, cut_cnf_file_name.length() - 1);
	std::cout << "cut_cnf_file_name " << cut_cnf_file_name << std::endl;

	decomp_set_file_name = argv[2];
	std::cout << "decomp_set_file_name " << decomp_set_file_name << std::endl;

	tests_count = atoi(argv[3]);
	std::cout << "tests_count " << tests_count << std::endl;

	std::string str;
	if (argc > 4) {
		str = argv[4];
		if (str.find("sat") != std::string::npos)
			isSatSample = true;
	}
	std::cout << "isSatSample " << isSatSample << std::endl;
}

void makeSample::init()
{
	cnf_file.open(cnf_file_name.c_str());
	if (!cnf_file.is_open()) {
		std::cerr << "Error. !cnf_file.is_open()" << std::endl;
		exit(1);
	}

	bool isDecompSet = false;
	if (decomp_set_file_name != "no") {
		decomp_set_file.open(decomp_set_file_name.c_str());
		isDecompSet = true;
		if (!decomp_set_file.is_open()) {
			std::cerr << "Error. !cnf_file.is_open()" << std::endl;
			exit(1);
		}
	}
	
	if (tests_count <= 0) {
		std::cerr << "Error. tests_count <= 0" << std::endl;
		exit(1);
	}

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

	if (isDecompSet) {
		unsigned val;
		std::cout << "reading decomp_set ";
		while (decomp_set_file >> val) {
			std::cout << val << " ";
			decomp_set.push_back(val);
		}
	}
	std::cout << std::endl;
	std::cout << "decomp_set.size() " << decomp_set.size() << std::endl;

	mpi_b.isMakeSatSampleAnyWay = true;
	mpi_b.input_cnf_name = cut_cnf_file_name;
	mpi_b.ReadIntCNF();
	std::cout << "mpi_b.var_count " << mpi_b.var_count << std::endl;
	std::cout << "mpi_b.clause_count " << mpi_b.clause_count << std::endl;

	unsigned new_clause_count;
	std::vector< std::vector<bool> > plain_text_vec_vec;

	if (isSatSample) {
		mpi_b.cnf_in_set_count = tests_count;
		mpi_b.MakeSatSample(state_vec_vec, stream_vec_vec, plain_text_vec_vec, 0);
		new_clause_count = mpi_b.clause_count + decomp_set.size() + stream_vec_vec[0].size();
	}
	else
		new_clause_count = mpi_b.clause_count + decomp_set.size();

	head_cnf_sstream << "p cnf " << mpi_b.var_count << " " << new_clause_count << std::endl;

	decomp_set_file.close();
	cnf_file.close();
}

void makeSample::makeUnsatSample()
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

void makeSample::makeSatSample()
{
	unsigned cur_var_ind;
	unsigned cur_stream_index;
	std::stringstream oneliteral_sstream, current_name_sstream;
	for (unsigned i = 0; i < tests_count; i++) {
		for (std::vector<unsigned>::iterator it = decomp_set.begin(); it != decomp_set.end(); it++) {
			cur_var_ind = (*it) - 1;
			if (!(state_vec_vec[i][cur_var_ind]))
				oneliteral_sstream << "-";
			oneliteral_sstream << cur_var_ind + 1 << " 0" << std::endl;
		}
		cur_stream_index = 0;
		for (std::vector<bool>::iterator it = stream_vec_vec[i].begin(); it != stream_vec_vec[i].end(); it++) {
			cur_var_ind = (mpi_b.var_count - mpi_b.keystream_len) + cur_stream_index;
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