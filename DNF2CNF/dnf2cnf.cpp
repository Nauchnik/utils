#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 3;
	argv[1] = "bivium_template.cnf";
	argv[2] = "var_values.txt";
#endif
	if ( argc < 3 ) {
		std::cerr << "Usage: template_cnf var_values" << std::endl;
		return 1;
	}
	std::string ifile_name = argv[1];
	std::cout << "Opening " << ifile_name << std::endl;
	std::ifstream ifile( ifile_name.c_str() );
	std::string str;
	std::string prefix;
	std::size_t found;
	std::stringstream sstream;
	unsigned template_cnf_varibales = 0;
	
	// read count of variables in template CNF
	prefix = "p cnf ";
	unsigned processed_string_count = 0;
	while ( getline( ifile, str ) ) {
		found = str.find( prefix );
		if ( found != std::string::npos ) {
			str.erase( found, prefix.size() );
			sstream << str;
			sstream >> template_cnf_varibales;
			break;
		}
		processed_string_count++;
		if ( processed_string_count > 10 )
			break;
	}
	ifile.close();
	
	if ( !template_cnf_varibales ) {
		std::cerr << "template_cnf_varibales == 0" << std::endl;
		return 1;
	}
	
	// read decomposition varibales
	ifile_name = argv[2];
	ifile.open( ifile_name.c_str() );
	getline( ifile, str );
	prefix = "c var_set ";
	found = str.find( prefix );
	if ( found == std::string::npos ) {
		std::cerr << "'c input variables ' wasn't found in 1st string" << std::endl;
		return 1;
	}
	str.erase( found, prefix.size() );
	std::vector<unsigned> var_vec;
	sstream.clear(); sstream.str("");
	sstream << str;
	unsigned uint;
	while ( sstream >> uint )
		var_vec.push_back( uint );
	
	// read valus of decomposition variables
	std::vector<std::string> values_vec;
	while ( getline( ifile, str ) )
		values_vec.push_back( str );
	
	ifile.close();
	unsigned new_cnf_variables = values_vec.size();
	std::vector<unsigned> new_cnf_variables_vec;
	for ( unsigned i=0; i < values_vec.size(); i++ )
		new_cnf_variables_vec.push_back( template_cnf_varibales + i + 1 );
	
	std::stringstream from_dnf_sstream;
	// write long clause with all new variables in positive phase
	for ( auto &x : new_cnf_variables_vec )
		from_dnf_sstream << x << " ";
	from_dnf_sstream << "0" << std::endl;
	
	// write clauses A == B -> (A or not(B)) and ( not(A) or B ) where A is a new variable
	for ( unsigned i=0; i < values_vec.size(); i++ ) {
		// clause with positive phase of new variable
		from_dnf_sstream << new_cnf_variables_vec[i] << " ";
		for ( unsigned j=0; j < values_vec[i].size(); j++ ) {
			if ( values_vec[i][j] == '1' ) // negative phase cause De Morgana rule
				from_dnf_sstream << "-";
			from_dnf_sstream << var_vec[j] << " ";
		}
		from_dnf_sstream << "0" << std::endl;
		for ( unsigned j=0; j < values_vec[i].size(); j++ ) {
			from_dnf_sstream << "-" << new_cnf_variables_vec[i] << " "; // negative phase cause De Morgana rule
			if ( values_vec[i][j] == '0' ) 
				from_dnf_sstream << "-";
			from_dnf_sstream << var_vec[j] << " 0" << std::endl;
		}
	}
	
	std::ofstream from_dnf_file( "from_dnf" );
	from_dnf_file << from_dnf_sstream.rdbuf();
	from_dnf_file.close();
	
	return 0;
}