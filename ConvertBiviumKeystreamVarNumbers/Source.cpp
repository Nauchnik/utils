#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include "../../pdsat/src_common/win_headers/dirent.h"
#else
#include <dirent.h>
#endif

const std::string GoS_path = "./GoS";
const std::string Transalg_path = "./Transalg";

bool getdir( std::string dir, std::vector<std::string> &files );
std::vector<bool> get_keystream_values_from_file( std::string keystream_file_name, std::string generator_type );
std::vector<bool> get_reg_values_from_file( std::string reg_file_name, std::string generator_type );

int main( int argc, char **argv ) 
{
#ifdef _DEBUG
	argc = 2;
	argv[1] = "bivium_template_new.cnf";
#endif
	std::string template_file_name;
	std::stringstream convert_sstream;
	if ( argc != 2 ) {
		std::cerr << "Usage: [Transalg CNF template file]";
		return 1;
	}
	
	std::vector<std::string> GoS_files;
	getdir( GoS_path, GoS_files );
	std::vector<std::vector<bool>> reg_values_vec, keystream_values_vec;
	std::string generator_type;
	std::cout << "GoS_files.size() " << GoS_files.size() << std::endl;
	reg_values_vec.resize( GoS_files.size() / 2 );
	keystream_values_vec.resize( GoS_files.size() / 2 );
	std::cout << "reg_values_vec.size() " << reg_values_vec.size() << std::endl;
	
	int num;
	std::size_t found1, found2;
	std::string str_tmp;
	for ( auto &file_name : GoS_files ) {
		found1 = file_name.find( ".cnf" );
		found2 = file_name.find( ".output" );
		if ( file_name.find( "bivium" ) != std::string::npos ) {
			generator_type = "Bivium";
			if ( found1 != std::string::npos ) {
				str_tmp = file_name.substr( found1-1, 1 );
				std::istringstream( str_tmp ) >> num;
				keystream_values_vec[num-1] = get_keystream_values_from_file( file_name, generator_type );
			}
			else if ( found2 != std::string::npos ) {
				str_tmp = file_name.substr( found2-1, 1 );
				std::istringstream( str_tmp ) >> num;
				reg_values_vec[num-1] = get_reg_values_from_file( file_name, generator_type );
			}
		}
	}
	
	template_file_name  = argv[1];
	
	std::ifstream template_file( template_file_name.c_str() );
	if ( !template_file.is_open() ) {
		std:: cerr << "!template_file.is_open()" << std::endl;
		return 1;
	}
	std::stringstream template_clauses_sstream, template_header_comments_sstream;
	std::string str;
	
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
	
	
	int variables_count = 0;
	std::vector<int> result_keystream_variables;
	//unsigned keystream_variable_first = 578, keystream_variable_last = 777;
	unsigned keystream_variable_first = 443, keystream_variable_last = 642;
	for ( unsigned i=keystream_variable_first; i <= keystream_variable_last; i++ )
		result_keystream_variables.push_back(i);
	
	std::vector<int> result_reg_variables;
	unsigned reg_variable_first1 = 1, reg_variable_last1 = 177; 
	for ( unsigned i=reg_variable_first1; i <= reg_variable_last1; i++ )
		result_reg_variables.push_back(i);

	clauses_count += result_reg_variables.size();
	clauses_count += result_keystream_variables.size();

	if ( keystream_values_vec[0].size() != result_keystream_variables.size() ) {
		std::cerr << "keystream_values.size() != result_keystream_variables.size()" << std::endl;
		std::cerr << keystream_values_vec[0].size() << " != " << result_keystream_variables.size() << std::endl;
	}
	
	std::string out_file_name;
	std::ofstream out_file;
	std::string str_num;
	int k;
	for ( unsigned i=0; i < keystream_values_vec.size(); i++ ) {
		convert_sstream << i;
		convert_sstream >> str_num;
		convert_sstream.str(""); convert_sstream.clear();
		if ( generator_type == "Bivium" ) {
			out_file_name = Transalg_path + "/Transalg_Bivium_test" + str_num;
			out_file_name += ".cnf";
		}
		
		out_file.open( out_file_name.c_str() );
		out_file << "p cnf " << var_count << " " << clauses_count << std::endl;
		out_file << template_header_comments_sstream.str();
		k = 0;
		for ( auto &x : reg_values_vec[i] ) {
			convert_sstream << (x ? "" : "-") << result_reg_variables[k++] << " 0";
			out_file << convert_sstream.str() << std::endl;
			convert_sstream.str(""); convert_sstream.clear();
		}
		k = 0;
		for ( auto &x : keystream_values_vec[i] ) {
			convert_sstream << (x ? "" : "-") << result_keystream_variables[k++] << " 0";
			out_file << convert_sstream.str() << std::endl;
			convert_sstream.str(""); convert_sstream.clear();
		}
		out_file << template_clauses_sstream.str();
		out_file.close(); out_file.clear();
	}
	
	return 0;
}

std::vector<bool> get_keystream_values_from_file( std::string keystream_file_name, std::string generator_type )
{
	keystream_file_name = GoS_path + "/" + keystream_file_name;
	std::ifstream keystream_file( keystream_file_name.c_str() );
	if ( !keystream_file.is_open() ) {
		std:: cerr << "!keystream_file.is_open()" << std::endl;
		exit(1);
	}
	std::vector<bool> keystream_values;
	std::string str;
	std::stringstream sstream;
	int val;
	std::vector<int> vec_int;

	while ( getline( keystream_file, str ) ) {
		sstream << str;
		while ( sstream >> val )
			vec_int.push_back( val );
		if ( ( str.find( " 0" ) != std::string::npos ) && ( vec_int.size() == 2 ) )
			keystream_values.push_back( str[0] == '-' ? false : true );
		sstream.clear();
		sstream.str("");
		vec_int.clear();
	}

	keystream_file.close();
	return keystream_values;
}

std::vector<bool> get_reg_values_from_file( std::string reg_file_name, std::string generator_type )
{
	reg_file_name = GoS_path + "/" + reg_file_name;
	std::ifstream reg_file( reg_file_name.c_str() );
	if ( !reg_file.is_open() ) {
		std:: cerr << "!keystream_file.is_open()" << std::endl;
		exit(1);
	}
	std::vector<bool> reg_values, tmp_reg_values;
	std::string str;
	unsigned reg1_len, reg2_len;
	if ( generator_type == "Bivium" ) {
		reg1_len = 93;
		reg2_len = 84;
	}

	while ( getline( reg_file, str ) ) {
		if ( str.find( "true" ) != std::string::npos )
			reg_values.push_back( true );
		else if ( str.find( "false" ) != std::string::npos )
			reg_values.push_back( false );
		// reverse reg bits from GoS to Transalg
		if ( reg_values.size() == reg1_len  ) {
			tmp_reg_values = reg_values;
			reg_values.clear();
			for ( std::vector<bool>::reverse_iterator r_it = tmp_reg_values.rbegin(); r_it != tmp_reg_values.rend(); r_it++ )
				reg_values.push_back(*r_it);
		}
		if ( reg_values.size() == reg1_len + reg2_len  ) {
			tmp_reg_values = reg_values;
			reg_values.resize(reg1_len);
			unsigned k=0;
			for ( std::vector<bool>::reverse_iterator r_it = tmp_reg_values.rbegin(); r_it != tmp_reg_values.rend(); r_it++ ) {
				reg_values.push_back(*r_it);
				k++;
				if ( k == reg2_len  )
					break;
			}
		}
	}
	reg_file.close();
	return reg_values;
}

bool getdir( std::string dir, std::vector<std::string> &files )
{
    DIR *dp;
	std::string cur_name;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cout << std::endl << "Error in opening " << dir << std::endl;
        return false;
    }
    while ((dirp = readdir(dp)) != NULL) { 
		cur_name = std::string(dirp->d_name);
		if ( cur_name[0] != '.' ) files.push_back(cur_name); 
	}
    closedir(dp);
    return true;
}