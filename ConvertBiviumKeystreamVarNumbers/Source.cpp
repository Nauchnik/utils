#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

int main( int argc, char **argv ) 
{
#ifdef _DEBUG
	argc = 4;
	argv[1] = "bivium_template_new.cnf";
	//argv[1] = "GoS_bivium-0-200_template.cnf";
	argv[2] = "bivium-0-200_stream";
	argv[3] = "bivium-0-200.output";
#endif
	std::string template_file_name;
	std::string keystream_file_name;
	std::string reg_file_name;
	std::stringstream convert_sstream;
	if ( argc != 4 ) {
		std::cerr << "Usage: [CNF template file] [Bivium keystream file]";
		return 1;
	}
	
	template_file_name  = argv[1];
	keystream_file_name = argv[2];
	reg_file_name       = argv[3];
	
	std::ifstream template_file( template_file_name.c_str() );
	if ( !template_file.is_open() ) {
		std:: cerr << "!template_file.is_open()" << std::endl;
		return 1;
	}
	std::stringstream template_clauses_sstream, template_header_comments_sstream;
	std::string str;
	int variables_count = 0;
	std::vector<int> result_keystream_variables;
	//unsigned keystream_variable_first = 578, keystream_variable_last = 777;
	unsigned keystream_variable_first = 443, keystream_variable_last = 642;
	for ( unsigned i=keystream_variable_first; i <= keystream_variable_last; i++ )
		result_keystream_variables.push_back(i);
	
	std::vector<int> result_reg_variables;
	//unsigned reg_variable_first1 = 1, reg_variable_last1 = 93; 
	unsigned reg_variable_first1 = 1, reg_variable_last1 = 177; 
	for ( unsigned i=reg_variable_first1; i <= reg_variable_last1; i++ )
		result_reg_variables.push_back(i);
	/*unsigned reg_variable_first2 = 294, reg_variable_last2 = 377;
	for ( unsigned i=reg_variable_first2; i <= reg_variable_last2; i++ )
		result_reg_variables.push_back(i);*/
	
	bool isClausesStrings = false;
	bool isFirstClauseString = true;
	int var_count = 0, clauses_count = 0;
	std::stringstream str_sstream;
	int val;
	while ( getline( template_file, str ) ) {
		if ( str[0] == 'p' )
			continue;
		if ( ( str[0] == 'c' ) && ( !isClausesStrings ) )
			template_header_comments_sstream << str << std::endl;
		else {
			if ( !isFirstClauseString )
				template_clauses_sstream << std::endl;
			template_clauses_sstream << str;
			isClausesStrings = true;
			isFirstClauseString = false;
			if ( str[0] != 'c' )
				clauses_count++;
			str_sstream << str;
			while ( str_sstream >> val )
				if ( abs(val) > var_count )
					var_count = abs(val);
			str_sstream.clear(); str_sstream.str("");
		}
	}
	template_file.close();
	
	std::vector<bool> keystream_values;
	std::ifstream keystream_file( keystream_file_name.c_str() );
	if ( !keystream_file.is_open() ) {
		std:: cerr << "!keystream_file.is_open()" << std::endl;
		return 1;
	}

	while ( getline( keystream_file, str ) ) {
		if ( str[0] == 'c' ) continue;
		convert_sstream << str;
		while ( convert_sstream >> val ) {
			if ( !val ) 
				continue;
			else {
				keystream_values.push_back( (val > 0) ? true : false );
				break;
			}
		}
		convert_sstream.str(""); convert_sstream.clear();
	}
	keystream_file.close();
	std::ifstream reg_file( reg_file_name.c_str() );
	if ( !reg_file.is_open() ) {
		std:: cerr << "!keystream_file.is_open()" << std::endl;
		return 1;
	}

	/*while ( reg_file >> val ) {
		if ( !val ) continue;
		reg_values.push_back( (val > 0) ? true : false );
	}*/ 
	
	std::vector<bool> reg_values, tmp_reg_values;
	while ( getline( reg_file, str ) ) {
		if ( str.find( "true" ) != std::string::npos )
			reg_values.push_back( true );
		else if ( str.find( "false" ) != std::string::npos )
			reg_values.push_back( false );
		// reverse reg bits from GoS to Transalg
		if ( reg_values.size() == 93 ) {
			tmp_reg_values = reg_values;
			reg_values.clear();
			for ( std::vector<bool>::reverse_iterator r_it = tmp_reg_values.rbegin(); r_it != tmp_reg_values.rend(); r_it++ )
				reg_values.push_back(*r_it);
		}
		if ( reg_values.size() == 177 ) {
			tmp_reg_values = reg_values;
			reg_values.resize(93);
			unsigned k=0;
			for ( std::vector<bool>::reverse_iterator r_it = tmp_reg_values.rbegin(); r_it != tmp_reg_values.rend(); r_it++ ) {
				reg_values.push_back(*r_it);
				k++;
				if ( k == 84 )
					break;
			}
		}
	}
	reg_file.close();
	
	if ( keystream_values.size() != result_keystream_variables.size() ) {
		std::cerr << "keystream_values.size() != result_keystream_variables.size()" << std::endl;
		std::cerr << keystream_values.size() << " != " << result_keystream_variables.size() << std::endl;
	}
	
	clauses_count += result_reg_variables.size();
	clauses_count += result_keystream_variables.size();
	
	std::string out_file_name = "out.cnf";
	std::ofstream out_file( out_file_name.c_str() );
	out_file << "p cnf " << var_count << " " << clauses_count << std::endl;
	out_file << template_header_comments_sstream.rdbuf();
	for ( unsigned i = 0; i < reg_values.size(); i++ ) {
		convert_sstream << (reg_values[i] ? "" : "-") << result_reg_variables[i] << " 0";
		out_file << convert_sstream.str() << std::endl;
		convert_sstream.str(""); convert_sstream.clear();
	}
	for ( unsigned i = 0; i < keystream_values.size(); i++ ) {
		convert_sstream << (keystream_values[i] ? "" : "-") << result_keystream_variables[i] << " 0";
		out_file << convert_sstream.str() << std::endl;
		convert_sstream.str(""); convert_sstream.clear();
	}
	out_file << template_clauses_sstream.rdbuf();
	out_file.close();
	
	return 0;
}