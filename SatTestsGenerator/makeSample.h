#ifndef MAKE_SAMPLE_H
#define MAKE_SAMPLE_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "mpi_base.h"
#include "addit_func.h"

class makeSample
{
public:
	makeSample();
	void readInput(int argc, char **argv);
	void init();
	bool ifSatSample();
	void makeUnsatSample();
	void makeSatSample();
	boost::random::mt19937 gen;
private:
	unsigned tests_count;
	std::string cnf_file_name;
	std::string cut_cnf_file_name;
	std::string decomp_set_file_name;
	std::string cnf_name_common_part;
	std::vector<std::ofstream*> test_cnf_files;
	std::vector<unsigned> decomp_set;
	bool isSatSample;
	std::ifstream cnf_file;
	std::ifstream decomp_set_file;
	std::stringstream comment_cnf_sstream;
	std::stringstream head_cnf_sstream;
	std::stringstream main_cnf_sstream;
	std::vector<std::vector<bool>> state_vec_vec, stream_vec_vec;
	MPI_Base mpi_b;
};

#endif