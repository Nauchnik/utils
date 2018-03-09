#ifndef MAKE_SAMPLE_H
#define MAKE_SAMPLE_H

#include "mpi_base.h"
#include "addit_func.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define RANDOM_UNSAT_SAMPLE 0
#define RANDOM_SAT_SAMPLE 1
#define ASSUMPTIONS_SAMPLE 2
#define INPUT_OUTPUT_ASSUMPTIONS_SAMPLE 3
#define INPUT_ASSUMPTIONS_SAMPLE 4

class makeSample
{
public:
	makeSample();
	void readInput(int argc, char **argv);
	void init();
	void makeRandomUnsatSample();
	void makeRandomSatSample();
	void makeSampleFromAssumptions();
	void makeSampleFromInputOutputAssumptions();
	void makeSampleFromInputs();
	boost::random::mt19937 gen;
	short int launchType;
private:
	unsigned tests_count;
	string cnf_file_name;
	string cut_cnf_file_name;
	string decomp_set_file_name;
	string assumptions_file_name;
	string cnf_name_common_part;
	vector<ofstream*> test_cnf_files;
	vector<unsigned> decomp_set;
	vector<unsigned> decomp_set_indexes;
	vector<unsigned> input_variables;
	vector<unsigned> output_variables;
	ifstream cnf_file;
	ifstream decomp_set_file;
	stringstream comment_cnf_sstream;
	stringstream head_cnf_sstream;
	stringstream main_cnf_sstream;
	vector<vector<bool>> state_vec_vec, stream_vec_vec;
	string input_output_folder_name;
	MPI_Base mpi_b;
};

#endif