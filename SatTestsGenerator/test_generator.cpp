// Generator of SAT instances by Oleg Zaikin. 2013

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include "makeSample.h"

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 6;
	argv[1] = "./A51_TA64.cnf";
	argv[2] = "-input_output_assumptions";
	argv[3] = "A5_1_test_31_known";
	//argv[2] = "known_point";
	//argv[2] = "no"; // no decomp set - for generating the nonweakened instances
	//argv[3] = "2";
	//argv[4] = "-sat";
	//argv[4] = "-assumptions";
	//argv[5] = "assumptions";
#endif

	if (argc < 4) {
		std::cerr << "Usage: cnf_file {decomp_set_file | no | -input_output_assumptions} {tests_count | input_output_folder_name} [-sat | -assumptions ] [file_assumptions]";
		exit(1);
	}
	
	makeSample make_s;
	make_s.readInput( argc, argv );
	make_s.init();
	if ( make_s.launchType == RANDOM_UNSAT_SAMPLE )
		make_s.makeRandomUnsatSample();
	else if (make_s.launchType == RANDOM_SAT_SAMPLE)
		make_s.makeRandomSatSample();
	else if (make_s.launchType == ASSUMPTIONS_SAMPLE)
		make_s.makeSampleFromAssumptions();
	else if ( make_s.launchType == INPUT_OUTPUT_ASSUMPTIONS_SAMPLE )
		make_s.makeSampleFromInputOutputAssumptions();
	
	//system("pause");
	return 0;
}