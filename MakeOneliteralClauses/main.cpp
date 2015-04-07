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
	getline( ifile, str ); // read varibles of decomposition set	
	unsigned pos = str.find("-");
	unsigned var;
	if ( pos != std::string::npos ) { // interavl of variables
		unsigned from1 = 0;
		unsigned to1 = pos - 1;
		unsigned from2 = pos+1;
		unsigned to2 = str.size()-1;
		unsigned start_var, end_var;
		std::istringstream( str.substr( from1, to1 - from1 + 1) ) >> start_var;
		std::istringstream( str.substr( from2, to2 - from2 + 1) ) >> end_var;
		for ( unsigned var=start_var; var <= end_var; var++ )
			var_set.push_back(var);
	}
	else { // list of variables
		std::stringstream sstream;
		sstream << str;
		while ( sstream >> var )
			var_set.push_back(var);
	}
	std::vector<std::string> var_values_vec;
	std::stringstream sstream;
	std::string tmp_str;
	bool isMatchString;
	while ( getline( ifile, str ) ) { // read values of variables
		sstream << str;
		while ( sstream >> tmp_str ) {
			if ( tmp_str.size() < 5 )
				continue;
			isMatchString = true;
			for ( auto &x : tmp_str )
				if ( ( x != '1' ) && ( x != '0' ) ) {
					isMatchString = false;
					break;
				}
			if ( isMatchString )
				break;
		}
		var_values_vec.push_back( tmp_str );
		sstream.clear(); sstream.str("");
	}

	ifile.close();
	/*if ( str.size() < var_set[var_set.size() - 1] ) {
		std::cerr << "str.size() < var_set[var_set.size() - 1] : " << str.size() << " < " << var_set[var_set.size() - 1] << std::endl;
		return 1;
	}*/
	std::ofstream ofile;
	std::string ofile_name = "out";
	unsigned k = 0;
	for ( auto &x : var_values_vec ) {
		sstream << ofile_name << "_" << k;
		ofile.open( sstream.str().c_str() );
		for ( unsigned i=0; i < var_set.size(); i++ ) // write corresponding oneliteral clauses
			ofile << ( x[i] == '1' ? "" : "-") << var_set[i] << " 0" << std::endl;
		ofile.close();
		sstream.clear();
		sstream.str("");
		k++;
	}
	return 0;
}