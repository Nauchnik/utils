#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

int main( int argc, char *argv[] )
{
	//argc = 3;
	//argv[1] = "diag9_2.cnf";
	//argv[1] = "diag7_2.cnf";
	//argv[2] = "diag7_2.inc";
	//argv[1] = "test.cnf";
	//argv[2] = "diag9_2.inc";
	
	if ( ( argc < 3 ) || ( argc > 3 ) ) {
		cout << "Usage: cnf_file Inc_file" << endl;
		return 1;
	}

	string cnf_name = argv[1];
	string inc_name = argv[2];
	string str;
	stringstream sstream, inc_sstream;
	int int_val, lit_count, claues_count;
	ifstream cnf_file( cnf_name.c_str(), ios_base :: in );

	claues_count = 0;
	while ( getline( cnf_file, str ) )
	{
		if ( claues_count )
			inc_sstream << ", ";
		if ( ( str[0] == 'p' ) || ( str[0] == 'c' ) )
			continue;
		sstream << str;
		lit_count = 0;
		while ( !sstream.eof() ) 
		{
			if ( lit_count )
				inc_sstream << ", ";
			sstream >> int_val;
			inc_sstream << int_val;
			lit_count++;
			if ( int_val == 0 )
				break; // skip ' ' after 1st '0'
		}
		claues_count++;
		sstream.str(""); sstream.clear();
	}
	ofstream inc_file( inc_name.c_str(), ios_base :: out );
	inc_file << inc_sstream.rdbuf();
	cnf_file.close();
	inc_file.close();
	return 0;
}