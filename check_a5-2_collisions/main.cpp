#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include "a5_2.h"

using namespace std;

int main( int argc, char **argv )
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
	vector<string> vec_str;
	string decomp_set_name;
	vector<unsigned> decomp_set_vec;
	ifstream decomp_set_file;
#ifdef _DEBUG
	argc = 2;
	argv[1] = "set_a5-2_15vars.txt";
#endif
	unsigned uint;
	if ( argc > 1 ) {
		decomp_set_name = argv[1];
		cout << "opening " << decomp_set_name << endl;
		decomp_set_file.open( decomp_set_name.c_str() );
		while ( decomp_set_file >> uint )
			decomp_set_vec.push_back( uint );
		decomp_set_file.close();
	}
	while ( getline( infile, str ) ) {
		sstream << "literals_" << count++; 
		ofile_name = sstream.str();
		sstream.str(""); sstream.clear();
		vec_str.push_back( str );
		out_sstream << str << endl;
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
	unsigned hamming_distance = 0;
	if ( vec_str.size() > 1 )
	for ( unsigned i=0; i < vec_str[0].size(); i++ )
		if ( vec_str[0][i] != vec_str[1][i] )
			hamming_distance++;
	vector<unsigned > vec_weight;
	vec_weight.resize( vec_str.size() );
	out_sstream << "vec_weight" << endl;
	for ( unsigned i=0; i < vec_weight.size(); i++ ) {
		vec_weight[i] = 0;
		for ( unsigned j=0; j < vec_str[i].size(); j++ )
			if ( vec_str[i][j] == '1' ) vec_weight[i]++;
		out_sstream << vec_weight[i] <<  endl;
	}
	out_sstream << "set_1_matching" << endl;
	unsigned matching;
	if ( decomp_set_vec.size() > 0 ) {
		for ( unsigned i=0; i < vec_str.size(); i++ ) {
			matching = 0;
			for ( unsigned j=0; j < decomp_set_vec.size(); j++ )
				if ( vec_str[i][decomp_set_vec[j]-1] == '1' )
					matching++;
			out_sstream <<  matching << endl;
		}
	}
	out_sstream << "hamming_distance " << hamming_distance << endl;
	infile.close();
	cout << out_sstream.str();
	ofstream outfile("out.txt");
	outfile << out_sstream.rdbuf();
	outfile.close();
	return 0;
}