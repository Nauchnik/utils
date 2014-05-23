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
	int count = 0;
	string ofile_name;
	stringstream out_sstream;
	while ( getline( infile, str ) ) {
		sstream << "literals_" << count++; 
		ofile_name = sstream.str();
		sstream.str(""); sstream.clear();
		ofile.open( ofile_name.c_str() );
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
			out_sstream << stream_vec[i];
		}
		out_sstream << endl;
	}
	infile.close();
	cout << out_sstream.str();
	ofstream outfile("out.txt");
	outfile << out_sstream.rdbuf();
	outfile.close();
	return 0;
}