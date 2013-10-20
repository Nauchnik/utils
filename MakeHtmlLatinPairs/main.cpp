#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

void AddSquaresToHtml( vector< vector<string> > &squares, stringstream &html_code );

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 2;
	argv[1] = "in.txt";
	argv[2] = "out.txt";
#endif	

	if ( argc < 2 ) {
		cerr << "USAGE: prog [infile - list of triples of squares] [outfile - html code]" << endl;
		return 1;
	}
	ifstream infile( argv[1] );
	ofstream ofile( argv[2] );
	string cur_string;
	vector< vector<string> > squares;
	stringstream html_code;
	int square_index = 0;
	unsigned square_row_index = 0;
	while ( getline( infile, cur_string ) ) {
		if ( cur_string.size() == 0 ) {
			squares.resize( squares.size() + 1 ); // new square
			continue;
		}
		else if ( ( cur_string[0] == 'T' ) || ( cur_string[0] == 'S' ) ) { // start reading new set of squares
			if ( squares.size() > 0 ) { // if end of solution - write it to sstream
				squares.resize( squares.size() - 1 ); // remove last empty square
				AddSquaresToHtml( squares, html_code );
			}
			squares.resize( 0 );
		}
		else // reading row of square
			squares[squares.size()-1].push_back( cur_string );
	}
	AddSquaresToHtml( squares, html_code ); // add last set of squares
	infile.close();
	ofile << html_code.rdbuf();
	ofile.close();

	return 0; 
}

void AddSquaresToHtml( vector< vector<string> > &squares, stringstream &html_code )
{
	for ( unsigned j=0; j < squares[0].size(); ++j ) {
		for ( unsigned i=0; i < squares.size() - 1; ++i )
			html_code << squares[i][j] + "&nbsp;&nbsp ";
		html_code << squares[squares.size()-1][j] + "<br>" << endl;
	}
	html_code << endl;
}