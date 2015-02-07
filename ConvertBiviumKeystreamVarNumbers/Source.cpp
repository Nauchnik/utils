#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>

#ifdef _WIN32
#include "../../pdsat/src_common/win_headers/dirent.h"
#else
#include <dirent.h>
#endif

const std::string GoS_path = "./GoS";
const std::string result_path = "./ResultFiles";

bool getdir( std::string dir, std::vector<std::string> &files );
std::vector<bool> get_keystream_values_from_file( std::string keystream_file_name, std::string generator_type );
std::vector<bool> get_reg_values_from_file( std::string reg_file_name, std::string generator_type );
void get_cnf_data( std::string cnf_file_name, std::stringstream &cnf_header_comments_sstream,
				   std::stringstream &cnf_clauses_sstream, unsigned &cnf_vars_count, unsigned &cnf_clauses_count );

int main( int argc, char **argv ) 
{
#ifdef _DEBUG
	argc = 3;
	argv[1] = "Bivium_template_new.cnf";
	argv[2] = "60";
#endif
	std::string transalg_template_file_name;
	std::stringstream convert_sstream;
	if ( argc != 3 ) {
		std::cerr << "Usage: [Transalg CNF template file]";
		return 1;
	}

	int known_bits = atoi( argv[2] );
	
	std::vector<std::string> GoS_files;
	getdir( GoS_path, GoS_files );
	std::vector< std::vector<bool> > reg_values_vec, keystream_values_vec;
	std::string generator_type;
	std::cout << "GoS_files.size() " << GoS_files.size() << std::endl;
	reg_values_vec.resize( GoS_files.size() / 2 );
	keystream_values_vec.resize( GoS_files.size() / 2 );
	std::cout << "reg_values_vec.size() " << reg_values_vec.size() << std::endl;
	
	int num;
	std::size_t found1, found2;
	std::string str, old_GoS_file_name, new_GoS_file_name;
	std::ifstream old_GoS_file;
	std::ofstream new_GoS_file;
	std::stringstream GoS_cnf_sstream;
	std::string str_num;
	int pos = 0;
	for ( std::vector<std::string>::iterator file_it = GoS_files.begin(); file_it != GoS_files.end(); file_it++ ) {
		found1 = (*file_it).find( ".cnf" );
		found2 = (*file_it).find( ".output" );
		if ( (*file_it).find( "bivium" ) != std::string::npos ) {
			generator_type = "Bivium";
			if ( found1 != std::string::npos ) {
				str_num = (*file_it).substr( found1-1, 1 );
				pos = found1-2;
				while ( (*file_it)[pos] != '-' ) {
					str_num = (*file_it)[pos] + str_num;
					pos--;
				}
				std::istringstream( str_num ) >> num;
				keystream_values_vec[num-1] = get_keystream_values_from_file( (*file_it), generator_type );
				// save to file with short name
				old_GoS_file_name = GoS_path + "/" + (*file_it);
				old_GoS_file.open( old_GoS_file_name.c_str() );
				while ( getline( old_GoS_file, str ) )
					GoS_cnf_sstream << str << std::endl;
				old_GoS_file.close(); 
				old_GoS_file.clear();
				convert_sstream << num-1;
				new_GoS_file_name = result_path + "/GoS_Bivium_test" + convert_sstream.str();
				new_GoS_file_name += ".cnf";
				/*new_GoS_file.open( new_GoS_file_name.c_str()  );
				new_GoS_file << GoS_cnf_sstream.str();
				GoS_cnf_sstream.clear(); GoS_cnf_sstream.str("");
				new_GoS_file.close(); new_GoS_file.clear();*/
				convert_sstream.str(""); convert_sstream.clear();
			}
			else if ( found2 != std::string::npos ) {
				str_num = (*file_it).substr( found2-1, 1 );
				pos = found2-2;
				while ( (*file_it)[pos] != '-' ) {
					str_num = (*file_it)[pos] + str_num;
					pos--;
				}
				std::istringstream( str_num ) >> num;
				reg_values_vec[num-1] = get_reg_values_from_file( (*file_it), generator_type );
			}
		}
	}
	
	transalg_template_file_name  = argv[1];
	std::stringstream transalg_template_clauses_sstream, transalg_template_header_comments_sstream;
	unsigned transalg_cnf_vars_count, transalg_cnf_clauses_count;
	
	// get date from CNF: vars count, clauses count, clauses and comments
	get_cnf_data( transalg_template_file_name, transalg_template_header_comments_sstream,
				  transalg_template_clauses_sstream, transalg_cnf_vars_count, transalg_cnf_clauses_count );
	
	int variables_count = 0;
	std::vector<int> transalg_keystream_variables;
	//unsigned keystream_variable_first = 578, keystream_variable_last = 777;
	unsigned keystream_variable_first = 443, keystream_variable_last = 642;
	for ( unsigned i=keystream_variable_first; i <= keystream_variable_last; i++ )
		transalg_keystream_variables.push_back(i);
	
	std::vector<int> transalg_reg_variables;
	unsigned reg_variable_first1 = 1, reg_variable_last1 = 177; 
	for ( unsigned i=reg_variable_first1; i <= reg_variable_last1; i++ )
		transalg_reg_variables.push_back(i);

	transalg_cnf_clauses_count += known_bits;
	transalg_cnf_clauses_count += transalg_keystream_variables.size();

	if ( keystream_values_vec[0].size() != transalg_keystream_variables.size() ) {
		std::cerr << "keystream_values.size() != result_keystream_variables.size()" << std::endl;
		std::cerr << keystream_values_vec[0].size() << " != " << transalg_keystream_variables.size() << std::endl;
	}
	
	std::string transalg_out_file_name, info_file_name;
	std::stringstream str_num_sstream, known_bit_sstream;
	std::ofstream transalg_out_file, info_file;
	unsigned k;
	for ( unsigned i=0; i < keystream_values_vec.size(); i++ ) {
		str_num_sstream << i;
		if ( generator_type == "Bivium" ) {
			transalg_out_file_name = result_path + "/Transalg_Bivium_test" + str_num_sstream.str();
			transalg_out_file_name += "_last"; 
			known_bit_sstream << known_bits;
			transalg_out_file_name += known_bit_sstream.str();
			known_bit_sstream.str(""); known_bit_sstream.clear();
			transalg_out_file_name += "known";
			transalg_out_file_name += ".cnf";
			info_file_name = result_path + "/Bivium_test" + str_num_sstream.str();
			info_file_name += ".info";
		}
		str_num_sstream.str(""); str_num_sstream.clear();
		
		info_file.open( info_file_name.c_str() );
		info_file << "Input: ";
		for ( std::vector<bool>::iterator vb_it = reg_values_vec[i].begin(); vb_it != reg_values_vec[i].end(); vb_it++ )
			info_file << *vb_it ? "1" : "0";
		info_file << std::endl;
		info_file << "Output: ";
		for ( std::vector<bool>::iterator vb_it = keystream_values_vec[i].begin(); vb_it != keystream_values_vec[i].end(); vb_it++ )
			info_file << *vb_it ? "1" : "0";
		
		transalg_out_file.open( transalg_out_file_name.c_str() );
		transalg_out_file << "p cnf " << transalg_cnf_vars_count << " " << transalg_cnf_clauses_count << std::endl;
		transalg_out_file << transalg_template_header_comments_sstream.str();
		k = 0;
		for ( std::vector<bool>::iterator vb_it = reg_values_vec[i].begin(); vb_it != reg_values_vec[i].end(); vb_it++ ) {
			if ( k >= transalg_reg_variables.size() - known_bits ) { // write only literals for known bits
				convert_sstream << (*vb_it ? "" : "-") << transalg_reg_variables[k] << " 0";
				transalg_out_file << convert_sstream.str() << std::endl;
				convert_sstream.str(""); convert_sstream.clear();
			}
			k++;
		}
		k = 0;
		for ( std::vector<bool>::iterator vb_it = keystream_values_vec[i].begin(); vb_it != keystream_values_vec[i].end(); vb_it++ ) {
			convert_sstream << (*vb_it ? "" : "-") << transalg_keystream_variables[k++] << " 0";
			transalg_out_file << convert_sstream.str() << std::endl;
			convert_sstream.str(""); convert_sstream.clear();
		}
		transalg_out_file << transalg_template_clauses_sstream.str();
		transalg_out_file.close(); transalg_out_file.clear();
		info_file.close(); info_file.clear();
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

void get_cnf_data( std::string cnf_file_name, std::stringstream &cnf_header_comments_sstream,
				   std::stringstream &cnf_clauses_sstream, unsigned &cnf_vars_count, unsigned &cnf_clauses_count ) 
{
	std::ifstream cnf_file( cnf_file_name.c_str() );
	if ( !cnf_file.is_open() ) {
		std:: cerr << "!template_file.is_open()" << std::endl;
		exit(1);
	}
	
	cnf_vars_count = cnf_clauses_count = 0;
	bool isClausesStrings = false;
	bool isFirstClauseString = true;
	int transalg_var_count = 0, transalg_clauses_count = 0;
	std::stringstream str_sstream;
	int val;
	std::string str;
	while ( getline( cnf_file, str ) ) {
		if ( str[0] == 'p' )
			continue;
		if ( ( str[0] == 'c' ) && ( !isClausesStrings ) )
			cnf_header_comments_sstream << str << std::endl;
		else {
			if ( !isFirstClauseString )
				cnf_clauses_sstream << std::endl;
			cnf_clauses_sstream << str;
			isClausesStrings = true;
			isFirstClauseString = false;
			if ( str[0] != 'c' )
				cnf_clauses_count++;
			str_sstream << str;
			while ( str_sstream >> val )
				if ( abs(val) > transalg_var_count )
					cnf_vars_count = abs(val);
			str_sstream.clear(); str_sstream.str("");
		}
	}
	cnf_file.close();
}