#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

int main( int argc, char **argv)
{
	string infile_name, cur_str;

#ifdef _DEBUG
	argc = 2;
	argv[1] = "final_output.txt";
#endif
	if ( argc < 2 ) {
		cerr << "Usage: program [log file]" << endl;
		return 1;
	}
	infile_name = argv[1];
	
	ifstream infile;
	int sat_count = 0;
	string sat_start( " SAT " );
	string tmp_str;
	stringstream sstream;
	int cur_id;
	int start_index, end_index;
	ofstream outfile, sat_sets_file;
	outfile.open( "sat_id.txt", ios_base :: out );
	sat_sets_file.open( "sat_sets.txt", ios_base :: out );
	infile.open( infile_name.c_str(), ios_base :: in );
	cout << "Reading " << infile_name << endl;
	cout << "SAT sets" << endl; 
	while ( getline( infile, cur_str ) ) {
		sstream << cur_str;
		while ( !sstream.eof() ) {
			sstream >> tmp_str;
			if ( tmp_str == "id") {
				sstream >> cur_id;
				break;
			}
		}
		sstream.str(""); sstream.clear();
		start_index = cur_str.find( sat_start );
		if ( start_index != -1 ) {
			sat_count++;
			cout << "sat_count " << sat_count << endl;
			start_index += 5; " SAT ";
			cur_str = cur_str.substr( start_index, cur_str.size() - start_index );
			end_index = cur_str.find(" ");
			cur_str = cur_str.substr( 0, end_index );
			outfile << cur_id << endl;
			sat_sets_file << cur_str << endl << endl;
		}
	}
	infile.close();
	outfile.close();
	sat_sets_file.close();
	cout << "sat_count " << sat_count << endl;
	system("pause");
	return 0;
}