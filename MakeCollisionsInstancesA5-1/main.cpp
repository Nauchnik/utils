#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

int main()
{
	std::string fname;
#ifdef _DEBUG
	fname = "input.txt";
#endif
	std::ifstream ifile( fname );
	if ( !ifile.is_open() ) {
		std::cerr << "file " << fname << " open fails " << std::endl;
		return 1;
	}
	std::vector<unsigned> var_set;
	std::string str;
	getline( ifile, str );
	unsigned var;
	std::stringstream sstream;
	sstream << str;
	while ( sstream >> var )
		var_set.push_back(var);
	getline( ifile, str );
	ifile.close();
	std::ofstream ofile( "out" );
	for ( auto &x : var_set )
		ofile << (str[x-1] == '1' ? "" : "-") << x << " 0" << std::endl;
	ofile.close();
	return 0;
}