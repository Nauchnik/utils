#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif


using namespace std;

int getdir( string dir, vector<string> &files )
{
    DIR *dp;
	string cur_name;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << endl << "Error in opening " << dir;
        return 1;
    }
    while ((dirp = readdir(dp)) != NULL) { 
		cur_name = string(dirp->d_name);
		if ( cur_name[0] != '.' ) files.push_back(cur_name); 
	}
    closedir(dp);
    return 0;
}

int main( int argc, char** argv )
{
#ifdef _DEBUG
	argc = 3;
	argv[1] = "ls9_2.cnf";
	argv[2] = "assumptions";
#endif

	if ( argc < 3 ) {
		cerr << "Usage : [cnf_file] [assumptions_path]" << endl;
		exit;
	}

	string cnf_file_name = argv[1];
	string assumption_dir_name = argv[2];
	string str, cur_test_name, cnf_head, full_assumptions_file_name;
	ifstream cur_assumption_file;
	ofstream cur_test_file;
	stringstream main_clauses_sstream, assumptions_sstream;
	vector<string> assumption_files_names = vector<string>( );
	getdir( assumption_dir_name, assumption_files_names );
	
	// read main CNF - common part of every tests
	ifstream cnf_file( cnf_file_name );
	while ( getline( cnf_file, str ) ) {
		if ( str.length() < 2 ) 
		{ cout << "skipping " << str << endl; continue; }
		if ( str[0] == 'p' )
			cnf_head = str;
		else
			main_clauses_sstream << str << endl;
	}
	cnf_file.close();
	
	for ( unsigned i=0; i < assumption_files_names.size(); i++ ) {
		full_assumptions_file_name = "./" + assumption_dir_name + "/" + assumption_files_names[i];
		cur_assumption_file.open( full_assumptions_file_name );
		cout << "reading " << assumption_files_names[i] << endl;
		while ( getline( cur_assumption_file, str ) )
			if ( str.length() > 1 )	assumptions_sstream << str << endl;
		cur_assumption_file.close();
		cur_test_name = "test_" + cnf_file_name + "_" + assumption_files_names[i];
		cout << "making " << cur_test_name << endl;
		cur_test_file.open( cur_test_name.c_str() );
		cur_test_file << cnf_head << endl << assumptions_sstream.str() << main_clauses_sstream.str();
		cur_test_file.close();
		assumptions_sstream.str(""); assumptions_sstream.clear();
	}
	
	return 0;
}