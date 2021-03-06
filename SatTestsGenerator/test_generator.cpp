// Generator of SAT instances by Oleg Zaikin. 2013

#include "makeSample.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 5;
	argv[1] = "./a5_114bit.cnf";
	argv[2] = "64vars";
	argv[3] = "1";
	argv[4] = "-sat";
	//argv[2] = "known_point";
	//argv[2] = "no"; // no decomp set - for generating the nonweakened instances
	//argv[3] = "2";
	//argv[4] = "-sat";
	//argv[4] = "-assumptions";
	//argv[5] = "assumptions";
#endif

	if (argc < 4) {
		std::cerr << "Usage: "
		<< "Variant 1: cnf_file {decomp_set_file | no} tests_count [-sat | -assumptions ] [file_assumptions]"
		<< "Variant 2: cnf_file decomp_set_file -input_output_assumptions input_output_folder_name";
		exit(1);
	}
	
	makeSample make_s;
	make_s.readInput( argc, argv );
	make_s.init();
	std::cout << "init stage done" << std::endl;
	std::cout << "make_s.launchType " << make_s.launchType << std::endl;
	
	if ( make_s.launchType == RANDOM_UNSAT_SAMPLE )
		make_s.makeRandomUnsatSample();
	else if (make_s.launchType == RANDOM_SAT_SAMPLE)
		make_s.makeRandomSatSample();
	else if (make_s.launchType == ASSUMPTIONS_SAMPLE)
		make_s.makeSampleFromAssumptions();
	else if (make_s.launchType == INPUT_ASSUMPTIONS_SAMPLE)
		make_s.makeSampleFromInputs();
	else if ( make_s.launchType == INPUT_OUTPUT_ASSUMPTIONS_SAMPLE )
		make_s.makeSampleFromInputOutputAssumptions();
	
	return 0;
}