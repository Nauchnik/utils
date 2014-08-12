#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 5;
	argv[1] = "bivium_template.cnf";
	argv[2] = "set_35vars";
	argv[3] = "known_sat_sample";
	argv[4] = "2";
#endif
	if ( argc < 5 ) {
		std::cerr << "Usage: template_cnf var_set sat_sample assumptions_count" << std::endl;
		return 1;
	}
	std::string template_cnf_file_name = argv[1];
	std::cout << "Opening " << template_cnf_file_name << std::endl;
	std::ifstream template_cnf_file( template_cnf_file_name.c_str() );
	std::string str;
	std::string prefix;
	std::size_t found;
	std::stringstream sstream;
	unsigned template_cnf_varibales = 0;
	unsigned assumptions_number = atoi( argv[4] );
	
	// read count of variables in template CNF
	prefix = "p cnf ";
	unsigned processed_string_count = 0;
	while ( getline( template_cnf_file, str ) ) {
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
	template_cnf_file.close();
	
	if ( !template_cnf_varibales ) {
		std::cerr << "template_cnf_varibales == 0" << std::endl;
		return 1;
	}
	
	// read decomposition varibales
	std::string decomp_set_file_name = argv[2];
	std::cout << "Opening " << decomp_set_file_name << std::endl;
	std::ifstream decomp_set_file( decomp_set_file_name.c_str() );
	getline( decomp_set_file, str );
	std::vector<unsigned> decomp_set_vec;
	sstream.clear(); sstream.str("");
	sstream << str;
	unsigned uint;
	while ( sstream >> uint )
		decomp_set_vec.push_back( uint );
	decomp_set_file.close();
	sstream.str("");
	sstream.clear();

	// read valus of decomposition variables
	std::vector<std::vector<bool>> values_vec;
	std::string values_file_name = argv[3];
	std::ifstream values_file( values_file_name.c_str() );
	std::vector<std::vector<bool>> stream_vec_vec, state_vec_vec;
	std::vector<bool> stream_vec, state_vec;
	std::cout << "reading state and stream from file " << values_file_name << std::endl;
	bool isState = false, isStream = false;
	while( getline( values_file, str ) ) {
		if( str == "state" ) {
			std::cout << "state string found" << std::endl;
			isState = true;
		}
		else if ( str == "stream" ) {
			std::cout << "stream string found" << std::endl;
			isState = false;
			isStream = true;
		}
		else {
			if ( isState ) {
				for ( unsigned i=0; i < str.size(); i++ )
					state_vec.push_back( str[i] == '1' ? true : false );
				state_vec_vec.push_back( state_vec );
				state_vec.clear();
			}
			else if ( isStream ) {
				for ( unsigned i=0; i < str.size(); i++ )
					stream_vec.push_back( str[i] == '1' ? true : false );
				stream_vec_vec.push_back( stream_vec );
				stream_vec.clear();
			}
		}
	} 
	std::cout << "state_vec_vec.size() "  << state_vec_vec.size()  << std::endl;
	std::cout << "stream_vec_vec.size() " << stream_vec_vec.size() << std::endl;
	values_file.close();

	if ( assumptions_number > state_vec_vec.size() ) {
		assumptions_number = state_vec_vec.size();
		std::cout << "assumptions_number changed to " << assumptions_number << std::endl;
	}
	
	std::cout << "assumptions_number " << assumptions_number << std::endl;
	values_vec.resize( assumptions_number );
	// add values of variables from decomp set
	for ( unsigned i=0; i < assumptions_number; i++ )
		for ( auto &y : decomp_set_vec ) 
			values_vec[i].push_back( state_vec_vec[i][y-1] );
	// add values of known output variables
	for ( unsigned i=0; i < assumptions_number; i++ )
		for ( std::vector<bool> :: iterator it = stream_vec_vec[i].begin(); it != stream_vec_vec[i].end(); it++ ) 
			values_vec[i].push_back(*it);
	
	unsigned new_cnf_variables = values_vec.size();
	std::cout << "new_cnf_variables " << new_cnf_variables << std::endl;
	std::vector<unsigned> new_cnf_variables_vec;
	for ( unsigned i=0; i < values_vec.size(); i++ )
		new_cnf_variables_vec.push_back( template_cnf_varibales + i + 1 );
	
	unsigned new_cnf_clauses = 0;
	std::stringstream from_dnf_sstream;
	// write long clause with all new variables in positive phase
	for ( auto &x : new_cnf_variables_vec )
		from_dnf_sstream << x << " ";
	from_dnf_sstream << "0" << std::endl;
	new_cnf_clauses++;
	std::vector<unsigned> assumptions_var_set = decomp_set_vec;
	unsigned out_variables_count = stream_vec_vec[0].size();
	unsigned first_out_varible = template_cnf_varibales - out_variables_count + 1;
	for ( unsigned i=0; i < out_variables_count; i++ )
		assumptions_var_set.push_back( first_out_varible + i );
	
	// write clauses A == B -> (A or not(B)) and ( not(A) or B ) where A is a new variable
	for ( unsigned i=0; i < values_vec.size(); i++ ) {
		// clause with positive phase of new variable
		from_dnf_sstream << new_cnf_variables_vec[i] << " ";
		for ( unsigned j=0; j < values_vec[i].size(); j++ ) {
			if ( values_vec[i][j] ) // negative phase cause De Morgana rule
				from_dnf_sstream << "-";
			from_dnf_sstream << assumptions_var_set[j] << " ";
			
		}
		from_dnf_sstream << "0" << std::endl;
		new_cnf_clauses++;
		for ( unsigned j=0; j < values_vec[i].size(); j++ ) {
			from_dnf_sstream << "-" << new_cnf_variables_vec[i] << " "; // negative phase cause De Morgana rule
			if ( !(values_vec[i][j]) ) 
				from_dnf_sstream << "-";
			from_dnf_sstream << assumptions_var_set[j] << " 0" << std::endl;
			new_cnf_clauses++;
		}
	}

	std::cout << "new_cnf_clauses " << new_cnf_clauses << std::endl;

	std::stringstream comment_cnf_sstream, main_cnf_sstream, head_cnf_sstream;
	template_cnf_file.open( template_cnf_file_name.c_str() );
	unsigned template_cnf_var_count = 0, template_cnf_clause_count = 0;
	unsigned main_str_count = 0, comment_str_count = 0;
	while ( getline( template_cnf_file, str ) ) {
		str.erase( std::remove(str.begin(), str.end(), '\r'), str.end() );
		if ( str[0] == 'c' ) {
			comment_cnf_sstream << str << std::endl;
			comment_str_count++;
		}
		else if ( str[0] == 'p' ) {
			sstream << str;
			sstream >> str >> str >> template_cnf_var_count >> template_cnf_clause_count;
			sstream.str("");
			sstream.clear();
			head_cnf_sstream << "p cnf " << template_cnf_var_count + new_cnf_variables << " " << 
				                template_cnf_clause_count + new_cnf_clauses << std::endl;
		}
		else {
			if ( main_str_count )
				main_cnf_sstream << std::endl;
			main_cnf_sstream << str;
			main_str_count++;
		}
	}
	template_cnf_file.close();
	
	sstream << template_cnf_file_name << "_" << new_cnf_variables << "subproblems";
	std::string result_file_name = sstream.str();
	result_file_name.erase( std::remove(result_file_name.begin() + 2, result_file_name.end(), '.'), result_file_name.end() );
	std::cout << "result_file_name " << result_file_name << std::endl;
	sstream.str(""); sstream.clear();
	std::ofstream from_dnf_file( result_file_name.c_str() );
	if ( !from_dnf_file.is_open() ) {
		std::cerr << "!from_dnf_file.is_open()" << std::endl;
		return 1;
	}
	if ( comment_str_count > 0 )
		from_dnf_file << comment_cnf_sstream.rdbuf();
	from_dnf_file << head_cnf_sstream.rdbuf();
	from_dnf_file << from_dnf_sstream.rdbuf();
	from_dnf_file << main_cnf_sstream.rdbuf();
	from_dnf_file.close();
	
	return 0;
}