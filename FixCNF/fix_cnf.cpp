#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 2;
	argv[1] = "cnf";
#endif
	if ( argc < 2 ) {
		std::cerr << "Usage: cnffile" << std::endl;
		return 1;
	}
	std::string cnf_name = argv[1];
	std::cout << "cnf_name " << cnf_name << std::endl;
	std::fstream cnf_file( cnf_name.c_str() );
	if( !cnf_file.is_open() ) {
		std::cerr << "!cnf_file.is_open()" << std::endl;
		return 1;
	}
	
	std::string str;
	std::stringstream comment_cnf_sstream, main_cnf_sstream, sstream;
	comment_cnf_sstream << "";
	main_cnf_sstream << "";
	unsigned comment_str_count = 0, main_str_count = 0, var_count = 0, clause_count = 0;
	int lit;
	while ( getline(cnf_file, str) ) {
		str.erase( std::remove(str.begin(), str.end(), '\r'), str.end() );
		if ( str[0] == 'c' ) {
			comment_cnf_sstream << str << std::endl;
			comment_str_count++;
		}
		else if (str[0] != 'p') {
			if ( main_str_count )
				main_cnf_sstream << std::endl;
			main_cnf_sstream << str;
			main_str_count++;
			sstream << str;
			while ( sstream >> lit ) {
				if ( abs( lit ) > (int)var_count )
					var_count = abs( lit );
			}
			sstream.str(""); sstream.clear();
		}
	}
	clause_count = main_str_count;
	cnf_file.close();
	cnf_file.clear();

	std::string cnf_name_fixed = cnf_name + "_fixed";

	cnf_file.open( cnf_name_fixed.c_str(), std::ios_base::out );
	cnf_file << comment_cnf_sstream.str();
	cnf_file << "p cnf " << var_count << " " << clause_count << std::endl;
	cnf_file << main_cnf_sstream.rdbuf();
	cnf_file.close();

	return 0;
}