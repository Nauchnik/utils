#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream> 

using namespace std;

const char ch[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

int main( int argc, char **argv )
{
	ifstream input_file;
	string str;
	input_file.open(argv[1], ios :: in);
	getline( input_file, str );
	str = str + " ";
	string word = "";
	string hex_val = "";
	unsigned int k = 0, bool_val;
	int val, bit_count = 0, index = 0;
	while ( k < str.size() )
	{
		if ( str[k] != ' ' )
			word += str[k];
		else
		{
			val = atoi( word.c_str() );
			cout << val << endl;
			( val < 0 ) ? bool_val = 0 : bool_val = 1;
			cout << bool_val << endl;
			word = "";
			index += bool_val << ( 3 - bit_count );
			bit_count++;
			if ( bit_count == 4 )
			{
				hex_val += ch[index];
				//cout << ch[index] << endl;
				//cout << "index = " << index << endl;
				bit_count = 0;
				index = 0;
			}
		}
		k++;
	}
	cout << hex_val;
	return 0;
}

