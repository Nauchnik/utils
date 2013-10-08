#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main( int argc, char **argv )
{
	argc = 2;
	argv[1] = "in.txt";
	argv[2] = "out.txt";
	
	if ( argc < 2 ) {
		cerr << "USAGE: prog [infile - list of pairs of squares] [outfile - html code]" << endl;
		return 1;
	}
	ifstream infile( argv[1] );
	ofstream ofile( argv[2] );
	string cur_string;
	vector< vector<string> > html_code_string; // html codes
	bool IsReadingFirstSquare;
	unsigned square_row_index = 0;
	while ( getline( infile, cur_string ) ) {
		if ( ( cur_string.size() == 0 ) && ( IsReadingFirstSquare ) ) { // start of 2nd square
			IsReadingFirstSquare = false;
			square_row_index = 0;
		}
		else if ( cur_string.size() == 0 ) // end reading of current pair
			continue;
		else if ( cur_string[0] == 'S' ) { // start reading new pair
			IsReadingFirstSquare = true;
			square_row_index = 0;
			html_code_string.resize( html_code_string.size() + 1 );
			html_code_string[html_code_string.size() - 1].resize( 10 );
		}
		else { // reading row of square
			if ( IsReadingFirstSquare )
				html_code_string[ html_code_string.size() - 1 ][square_row_index++] += cur_string + "&nbsp;&nbsp ";
			else
				html_code_string[ html_code_string.size() - 1 ][square_row_index++] += cur_string + "<br>";
		}
	}
	infile.close();
	for ( unsigned i = 0; i < html_code_string.size(); i++ ) {
		ofile << "SAT" << i+1 << endl;
		for ( unsigned j = 0; j < html_code_string[i].size(); j++ )
			ofile << html_code_string[i][j] << endl;
		ofile << endl;
	}
	ofile.close();

	return 0; 
}