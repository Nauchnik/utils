#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include "a5_2.h"

using namespace std;

int main()
{
	string in_file_name = "in.txt";
	ifstream infile( in_file_name.c_str(), ios_base :: in );
	vector<bool> state_vec, stream_vec;
	string str;
	state_vec.resize(81);
	stream_vec.resize(128);
	a5_2 a5_obj;
	stringstream sstream;
	ofstream ofile;
	while ( getline( infile, str ) ) {
		ofile.open( "literals.txt" );
		for ( unsigned i=0; i<str.size(); i++ ) {
			state_vec[i] = ( str[i] == '1' ) ? true : false;
			if ( !state_vec[i] )
				sstream << "-";
			sstream << i+1 << " 0" << endl;
		}
		ofile << sstream.rdbuf();
		sstream.clear(); sstream.str("");
		ofile.close();
		a5_obj.setKey( state_vec );
		for ( unsigned i=0; i < stream_vec.size(); i++ ) {
			stream_vec[i] = a5_obj.getNextBit();
			cout << stream_vec[i];
		}
		cout << endl;
	}
	infile.close();
	return 0;
}