#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <chrono>
#include "addit_func.h"

using namespace Addit_func;

int main( int argc, char **argv )
{	
	std::vector<std::string> out_files_names = std::vector<std::string>();
	
	if ( !Addit_func::getdir( ".", out_files_names ) ) { return 1; }
	sort( out_files_names.begin(), out_files_names.end() );
	
	std::cout << std::endl << "solver_files_names :" << std::endl;
	for ( std::vector<std::string> :: iterator it = out_files_names.begin(); it != out_files_names.end(); it++ )
		std::cout << *it << std::endl;
	
	std::ifstream ifile;
	std::ofstream ofile;
	ofile.open( "ursa_out" );
	std::string str, before_time_str = "[Solving time: ", after_time_str = "s]";
	unsigned copy_from, copy_to;
	for ( unsigned i=0; i < out_files_names.size(); i++ ) {
		if ( out_files_names[i].find( "out_" ) == std::string::npos )
			continue;
		ifile.open( out_files_names[i].c_str() );
		std::cout << "opened file " << out_files_names[i] << std::endl;
		while ( getline( ifile, str ) ) {
			if ( str.find( "[Solving time: " ) != std::string::npos ) {
				copy_from = str.find( before_time_str ) + before_time_str.size();
				copy_to = str.find( after_time_str );
				str = str.substr( copy_from, copy_to-copy_from );
				std::cout << "time str " << str << std::endl;
				ofile << str << std::endl;
			}
		}
		ifile.close();
	}
	ofile.close();
		
	return 0;
}