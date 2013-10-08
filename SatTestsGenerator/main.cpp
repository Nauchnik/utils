// Generator of SAT instances by Oleg Zaikin. 2013

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

using namespace std;

boost::random::mt19937 gen(static_cast<unsigned int>(std::time(0)));

int bool_rand() {
	boost::random::uniform_int_distribution<> dist(0, 1 );
	return dist(gen);
}

int main( int argc, char **argv )
{
	// debug
	/*argc = 4;
	argv[1] = "./bivium_test_0.cnf";
	argv[2] = "decomp_set.txt";
	argv[3] = "10";*/

	if ( argc < 4 ) {
		cout << "Usage: [cnf_file] [decomp_set_file] tests_count";
		return 1;
	}

	int tests_count;
	string cnf_file_name;
	cnf_file_name = argv[1];
	
	ifstream cnf_file( cnf_file_name.c_str() );
	if ( !cnf_file.is_open() ) {
		cout << "Error. !cnf_file.is_open()" << endl;
		return 1;
	}

	while ( (cnf_file_name[0] == '.') || ( cnf_file_name[0] == '/') )
	    cnf_file_name = cnf_file_name.substr(1, cnf_file_name.length() - 1);

	cout << "new cnf_file_name " << cnf_file_name << endl;
	ifstream decomp_set_file( argv[2] );
	if ( !decomp_set_file.is_open() ) {
		cout << "Error. !cnf_file.is_open()" << endl;
		return 1;	
	}
	tests_count = atoi( argv[3] );
	if ( tests_count <= 0 ) {
		cout << "Error. tests_count <= 0" << endl;
		return 1;
	}

	stringstream head_cnf_sstream, main_cnf_sstream;
	string str;
	unsigned head_str_count = 0, main_str_count = 0;
	while ( getline( cnf_file, str ) ) {
		if ( ( str[0] == 'c' ) || ( str[0] == 'p' ) ) {
			head_cnf_sstream << str << endl;
			head_str_count++;
		}
		else {
			main_cnf_sstream << str << endl;
			main_str_count++;
		}
	}
	cout << "head_str_count " << head_str_count << endl;
	cout << "main_str_count " << main_str_count << endl;

	vector<int> decomp_set;
	int val;
	cout << "reading decomp_set ";
	while ( !decomp_set_file.eof() ) {
		decomp_set_file >> val; 
		cout << val << " ";
		decomp_set.push_back( val );
	}
	cout << endl;
	cout << "decomp_set.size() " << decomp_set.size() << endl; 

	decomp_set_file.close();
	
	vector<ofstream*> test_cnf_files;
	test_cnf_files.resize( tests_count );
	stringstream current_name_sstream;

	unsigned found = cnf_file_name.find( "." );
	string cnf_name_common_part;
	if ( found != string::npos )
		cnf_name_common_part = cnf_file_name.substr( 0, found );
	else 
		cnf_name_common_part = cnf_file_name;

	for ( unsigned i = 0; i < test_cnf_files.size(); i++ ) {
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new ofstream( current_name_sstream.str().c_str() );
		(*test_cnf_files[i]) << head_cnf_sstream.str(); // write head of cnf file
		// write oneliteral clauses
		for ( unsigned j = 0; j < decomp_set.size(); j++ ) {
			if ( bool_rand() )
				(*test_cnf_files[i]) << "-";	
			(*test_cnf_files[i]) << decomp_set[j] << " 0"<< endl;
		}
		(*test_cnf_files[i]) << main_cnf_sstream.str(); // write clauses of main cnf
		(*test_cnf_files[i]).close();
		delete test_cnf_files[i];
		current_name_sstream.clear(); current_name_sstream.str("");
	}
	cout << tests_count << " tests were created" << endl;

	cnf_file.close();
  
	//system("pause");
	return 0;
}