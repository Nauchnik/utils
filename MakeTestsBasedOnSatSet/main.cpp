#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

int main( int argc, char** argv )
{
	if ( argc < 3 )
		cerr << "Error. argc < 3" << endl; 

	stringstream orig_file_sstream, sstream;
	string str, cnf_header;
	string cnf_file_name = argv[1];

	ifstream cnf_file( cnf_file_name );
	while ( getline( cnf_file, str ) )
		if ( str[0] == 'p' )
			cnf_header = str;
		else
			orig_file_sstream << str << endl;

	cnf_file.close();
	cout << "orig file reading done" << endl;

	unsigned found = cnf_file_name.find( "." );
	string cnf_name_common_part;
	if ( found != string::npos )
		cnf_name_common_part = cnf_file_name.substr( 0, found );
	else 
		cnf_name_common_part = cnf_file_name;

	ifstream literal_file( argv[2] );
	ofstream cur_instance_file;
	unsigned count = 0;
	string cur_instance_name, cur_clause, addit_zeros;
	stringstream clause_sstream;
	unsigned uint;
	while ( getline( literal_file, cur_clause ) ) {
		if ( cur_clause[0] == '-' )
			continue;
		cout << "cur_clause " << cur_clause << endl;
		clause_sstream << cur_clause;
		count++;
		// for correct sotring of files in folder
		addit_zeros = "";
		if ( count < 100 )
			addit_zeros += "0";
		if ( count < 10 )
			addit_zeros += "0";

		sstream << cnf_name_common_part << "_test" << addit_zeros << count << ".cnf";
		cur_instance_name = sstream.str();
		sstream.str(""); sstream.clear();

		cur_instance_file.open( cur_instance_name );
		cur_instance_file << cnf_header << endl; 
		while ( !clause_sstream.eof() ) {
			clause_sstream >> uint;
			if ( uint != 0 )
				cur_instance_file << uint << " 0" << endl;
		}
		clause_sstream.str(""); clause_sstream.clear();
		cur_instance_file << orig_file_sstream.str();
		cur_instance_file.close();
	}
	
	literal_file.close();
	
	return 0;
}