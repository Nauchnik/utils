// Generator of SAT instances by Oleg Zaikin. 2013

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include "makeSample.h""

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 5;
	argv[1] = "./bivium_template.cnf";
	argv[2] = "decomp_set.txt";
	//argv[2] = "no"; // no decomp set - for generating the nonweakened instances
	argv[3] = "2";
	argv[4] = "-sat";
#endif

	if (argc < 4) {
		std::cerr << "Usage: cnf_file decomp_set_file|no tests_count [-sat]";
		exit(1);
	}
	
	makeSample make_s;
	make_s.readInput( argc, argv );
	make_s.init();
	if ( make_s.ifSatSample() )
		make_s.makeSatSample();
	else
		make_s.makeUnsatSample();

	//system("pause");
	return 0;
}