#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <boost/dynamic_bitset.hpp>
#include "../../pdsat/src_common/addit_func.h"

int main() {
	std::ifstream ifile("in.txt");
	std::vector<int> vars;
	int val;
	while ( ifile >> val )
		vars.push_back( val );
	ifile.close();

	ifile.open( "bivium_reg_0" );
	std::vector<bool> var_values;
	unsigned k=0;
	while ( ifile >> val ) {
		if ( val == 0 )
			continue;
		var_values.push_back( val > 0 ? true : false );
	}
	ifile.close();
	boost::dynamic_bitset<> d_b;
	for ( unsigned i=0; i < vars.size(); i++ ) {
		d_b.push_back( var_values[vars[i]-1] );
		std::cout << d_b[d_b.size()-1];
	}
	std::cout << std::endl;
	unsigned long long ull;
	ull = Addit_func::BitsetToUllong( d_b );
	std::cout << ull;

	return 0;
}
