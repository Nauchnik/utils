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

#include "mpi_base.h"

using namespace std;

boost::random::mt19937 gen(static_cast<unsigned int>(std::time(0)));

int bool_rand() {
	boost::random::uniform_int_distribution<> dist(0, 1 );
	return dist(gen);
}

class makeSample
{
public:
	makeSample();
	bool readInput( int argc, char **argv );
	bool init();
	bool ifSatSample();
	void makeUnsatSample();
private:
	unsigned tests_count;
	string cnf_file_name;
	string decomp_set_file_name;
	string cnf_name_common_part;
	vector<ofstream*> test_cnf_files;
	vector<int> decomp_set;
	bool isSatSample;
	ifstream cnf_file;
	ifstream decomp_set_file;
	stringstream comment_cnf_sstream;
	stringstream head_cnf_sstream;
	stringstream main_cnf_sstream;
	vector<vector<bool>> state_vec_vec, stream_vec_vec;
};

makeSample :: makeSample() :
	isSatSample (false)
{}

bool makeSample :: ifSatSample() { return isSatSample; }

bool makeSample :: readInput( int argc, char **argv )
{
	if ( argc < 4 ) {
		cout << "Usage: cnf_file decomp_set_file tests_count [-sat]";
		return false;
	}
	
	cnf_file_name = argv[1];
	cout << "cnf_file_name " << cnf_file_name << endl;
	while ( (cnf_file_name[0] == '.') || ( cnf_file_name[0] == '/') )
	    cnf_file_name = cnf_file_name.substr(1, cnf_file_name.length() - 1);
	cout << "new cnf_file_name " << cnf_file_name << endl;
	
	decomp_set_file_name = argv[2];
	cout << "decomp_set_file_name " << decomp_set_file_name << endl;

	tests_count = atoi( argv[3] );
	cout << "tests_count " << tests_count << endl;
	
	string str;
	if ( argc > 4 ) {
		str = argv[4];
		if ( str.find( "sat" ) != string::npos )
			isSatSample = true;
	}
	cout << "isSatSample " << isSatSample << endl;

	return true;
}

bool makeSample :: init()
{
	cnf_file.open( cnf_file_name.c_str() );
	if ( !cnf_file.is_open() ) {
		cout << "Error. !cnf_file.is_open()" << endl;
		return false;
	}
	
	decomp_set_file.open( decomp_set_file_name.c_str() );
	if ( !decomp_set_file.is_open() ) {
		cout << "Error. !cnf_file.is_open()" << endl;
		return false;	
	}

	if ( tests_count <= 0 ) {
		cout << "Error. tests_count <= 0" << endl;
		return false;
	}

	test_cnf_files.resize( tests_count );

	unsigned found = cnf_file_name.find( "." );
	cnf_name_common_part = (found != string::npos) ? cnf_file_name.substr( 0, found ) : cnf_file_name;
	cout << "cnf_name_common_part " << cnf_name_common_part << endl;

	unsigned comment_str_count = 0, main_str_count = 0;
	string str;
	while ( getline( cnf_file, str ) ) {
		if ( str[0] == 'c' ) {
			comment_cnf_sstream << str << endl;
			comment_str_count++;
		}
		else if ( str[0] != 'p' ) {
			if ( main_str_count )
				main_cnf_sstream << endl;
			main_cnf_sstream << str;
			main_str_count++;
		}
	}
	cout << "main_str_count " << main_str_count << endl;

	int val;
	cout << "reading decomp_set ";
	while ( !decomp_set_file.eof() ) {
		decomp_set_file >> val; 
		cout << val << " ";
		decomp_set.push_back( val );
	}
	cout << endl;
	cout << "decomp_set.size() " << decomp_set.size() << endl; 

	MPI_Base mpi_b;
	mpi_b.isMakeSatSampleAnyWay = true;
	mpi_b.input_cnf_name = new char[cnf_file_name.size() + 1];
	strcpy( mpi_b.input_cnf_name, cnf_file_name.c_str() );
	mpi_b.ReadIntCNF();
	delete[] mpi_b.input_cnf_name;
	cout << "mpi_b.var_count " << mpi_b.var_count << endl;
	cout << "mpi_b.clause_count "  << mpi_b.clause_count  << endl;
	
	unsigned new_clause_count;

	if ( isSatSample ) {
		mpi_b.cnf_in_set_count = tests_count;
		mpi_b.MakeSatSample( state_vec_vec, stream_vec_vec );
		new_clause_count = mpi_b.clause_count + state_vec_vec[0].size() + stream_vec_vec[0].size();
	}
	else
		new_clause_count = mpi_b.clause_count + decomp_set.size();
	
	head_cnf_sstream << "p cnf " << mpi_b.var_count << " " << new_clause_count << endl;
	
	decomp_set_file.close();
	cnf_file.close();

	return true;
}

void makeSample :: makeUnsatSample()
{
	stringstream current_name_sstream;
	for ( unsigned i = 0; i < test_cnf_files.size(); i++ ) {
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new ofstream( current_name_sstream.str().c_str() );
		(*test_cnf_files[i]) << comment_cnf_sstream.str(); // write comments of cnf file
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
}

int main( int argc, char **argv )
{
	// debug
	argc = 4;
	argv[1] = "./bivium_test_0.cnf";
	argv[2] = "decomp_set.txt";
	argv[3] = "2";

	makeSample make_s;
	make_s.readInput( argc, argv );
	make_s.init();
	if ( !make_s.ifSatSample() )
		make_s.makeUnsatSample();
	
	//system("pause");
	return 0;
}