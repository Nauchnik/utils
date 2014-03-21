#include <fstream>
#include <string>
#include <iostream>

using namespace std;

int main( int argc, char *argv[] )
{
	if ( argc < 2 ) {
		cout << "prog [filename to split]" << endl;
		return 1;
	}
	string known_assumptions_file_name = argv[1];
	cout << "opening " << known_assumptions_file_name << endl;
	ifstream known_assumptions_file( known_assumptions_file_name.c_str(), ios_base :: in | ios_base :: binary );
	
	known_assumptions_file.seekg( 0, known_assumptions_file.end );
	unsigned long long byte_length = known_assumptions_file.tellg();
	unsigned long long ul = 0;
	unsigned long long assumptions_count = (byte_length - 2)/sizeof(ul); // 2 is prefix length
	unsigned long long first_file_assumptions_count = assumptions_count / 2;
	unsigned long long second_file_assumptions_count = assumptions_count - first_file_assumptions_count;
	cout << "assumptions_count " << assumptions_count << endl;
	cout << "first_file_assumptions_count " << first_file_assumptions_count << endl;
	cout << "second_file_assumptions_count " << second_file_assumptions_count << endl;

	short int si;
	known_assumptions_file.seekg( 0, known_assumptions_file.beg );
	known_assumptions_file.clear();
	known_assumptions_file.read( (char*)&si, sizeof(si) ); // read header
	
	ofstream first_assumptions_file, second_assumptions_file;
	string first_assumptions_file_name  = known_assumptions_file_name + "_split1";
	string second_assumptions_file_name = known_assumptions_file_name + "_split2";
	
	cout << "making first split file" << endl;
	first_assumptions_file.open( first_assumptions_file_name.c_str(), ios_base :: out | ios_base :: binary );
	first_assumptions_file.write( (char*)&si, sizeof(si) );
	unsigned long long read_ul_count = 0;
	while ( (read_ul_count < first_file_assumptions_count) && known_assumptions_file.read( (char*)&ul, sizeof(ul) ) ) {
		first_assumptions_file.write( (char*)&ul, sizeof(ul) );
		read_ul_count++;
		if ( read_ul_count % 1000000 == 0 )
			cout << "read_ul_count " << read_ul_count << endl;
	}
	first_assumptions_file.close();
	
	cout << "making second split file" << endl;
	second_assumptions_file.open( second_assumptions_file_name.c_str(), ios_base :: out | ios_base :: binary );
	second_assumptions_file.write( (char*)&si, sizeof(si) );
	read_ul_count = 0;
	while ( (read_ul_count < second_file_assumptions_count) && known_assumptions_file.read( (char*)&ul, sizeof(ul) ) ) {
		second_assumptions_file.write( (char*)&ul, sizeof(ul) );
		read_ul_count++;
		if ( read_ul_count % 1000000 == 0 )
			cout << "read_ul_count " << read_ul_count << endl;
	}
	known_assumptions_file.close();
	second_assumptions_file.close();

	cout << "done" << endl;
	
	return 0;
}