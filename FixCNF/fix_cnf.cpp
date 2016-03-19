#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

const unsigned CLAUSES_BLOCK_SIZE = 100000;
const unsigned MAX_BLOCKS_NUMBER = 1000;

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 2;
	argv[1] = "PODLS_known_DLS_0_1.cnf";
#endif
	if ( argc < 2 ) {
		std::cerr << "Usage: CNF file" << std::endl;
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
	std::stringstream comment_cnf_sstream, sstream;
	std::vector<std::stringstream*> main_cnf_sstream_vec;
	main_cnf_sstream_vec.resize(CLAUSES_BLOCK_SIZE);
	for (auto &x : main_cnf_sstream_vec)
		x = new std::stringstream();
	comment_cnf_sstream << "";
	*(main_cnf_sstream_vec[0]) << "";
	unsigned cur_clauses_number = 0, block_index = 0;
	unsigned comment_str_count = 0, var_count = 0, clause_count = 0;
	int lit;
	int pos;
	while ( getline(cnf_file, str) ) {
		if (str == "")
			continue;
		str.erase( std::remove(str.begin(), str.end(), '\r'), str.end() );
		pos = str.find(" 0 ");
		if ( pos != std::string::npos )
			str.resize(pos+2);
		if ( str[0] == 'c' ) {
			comment_cnf_sstream << str << std::endl;
			comment_str_count++;
		}
		else if (str[0] != 'p') {
			if ( (cur_clauses_number != 0) || ( block_index != 0 ) )
				*(main_cnf_sstream_vec[block_index]) << std::endl;
			*(main_cnf_sstream_vec[block_index]) << str;
			cur_clauses_number++;
			// add full current block to vector
			if (cur_clauses_number == CLAUSES_BLOCK_SIZE) {
				cur_clauses_number = 0;
				block_index++;
			}

			sstream << str;
			while ( sstream >> lit ) {
				if ( abs( lit ) > (int)var_count )
					var_count = abs( lit );
			}
			sstream.str(""); sstream.clear();
		}
	}
	*(main_cnf_sstream_vec[block_index]) << " "; // add space on the last string for treengeling
	std::cout << "CNF processed" << std::endl;
	std::cout << "clauses_block_size " << CLAUSES_BLOCK_SIZE << std::endl;
	std::cout << "block_index " << block_index << std::endl;
	std::cout << "clauses_number " << cur_clauses_number << std::endl;
	clause_count = CLAUSES_BLOCK_SIZE*block_index + cur_clauses_number;
	cnf_file.close();
	cnf_file.clear();
	
	// delete old data from old file and write updated data to it
	cnf_file.open( cnf_name.c_str(), std::ios_base::out );
	//cnf_file.open( "out.txt", std::ios_base::out );
	cnf_file << comment_cnf_sstream.str();
	cnf_file << "p cnf " << var_count << " " << clause_count << std::endl;
	for (auto x : main_cnf_sstream_vec)
		cnf_file << (*x).rdbuf();
	cnf_file.close();

	return 0;
}