#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <algorithm>

#ifdef _WIN32
#include "../../pdsat/src_common/win_headers/dirent.h"
#else
#include <dirent.h>
#endif

const std::string GoS_path = "./GoS";
const std::string result_path = "./ResultFiles";
std::string generator_type;
int instances_count = 0;

struct cnf_data
{
	std::string file_name;
	unsigned vars_count;
	unsigned clauses_count;
	std::stringstream header_comments_sstream;
	std::stringstream clauses_sstream;
};

bool getdir( std::string dir, std::vector<std::string> &files );
std::vector<bool> get_keystream_values_from_file( std::string keystream_file_name );
std::vector<bool> get_reg_values_from_file( std::string reg_file_name, std::string generator_type );
void get_cnf_data( cnf_data &cnf );

int main( int argc, char **argv ) 
{
#ifdef _DEBUG
	argc = 3;
	argv[1] = "trivium_300_template_new.cnf";
	argv[2] = "288";
#endif
	std::string transalg_template_file_name;
	std::stringstream convert_sstream;
	if ( argc != 3 ) {
		std::cerr << "Usage: [Transalg CNF template file]";
		return 1;
	}

	unsigned transalg_reg_variable_first = 0, transalg_reg_variable_last = 0; 
	unsigned transalg_keystream_variable_first = 0, transalg_keystream_variable_last = 0; 
	unsigned GoS_reg1_variable_first = 0, GoS_reg1_variable_last = 0;
	unsigned GoS_reg2_variable_first = 0, GoS_reg2_variable_last = 0;
	unsigned GoS_reg3_variable_first = 0, GoS_reg3_variable_last = 0;
	
	cnf_data transalg_template_cnf;
	transalg_template_cnf.file_name = argv[1];
	if ( transalg_template_cnf.file_name.find( "bivium" ) != std::string::npos ) {
		generator_type = "bivium";
		transalg_reg_variable_first = 1;
		transalg_reg_variable_last = 177;
		transalg_keystream_variable_first = 443;
		transalg_keystream_variable_last = 642;
		GoS_reg1_variable_first = 1;
		GoS_reg1_variable_last = 93;
		GoS_reg2_variable_first = 294;
		GoS_reg2_variable_last = 377;
	}
	else if ( transalg_template_cnf.file_name.find( "trivium" ) != std::string::npos ){
		generator_type = "trivium";
		transalg_reg_variable_first = 1;
		transalg_reg_variable_last = 288;
		transalg_keystream_variable_first = 988;
		transalg_keystream_variable_last = 1287;
		GoS_reg1_variable_first = 1;
		GoS_reg1_variable_last = 93;
		GoS_reg2_variable_first = 394;
		GoS_reg2_variable_last = 477;
		GoS_reg3_variable_first = 778;
		GoS_reg3_variable_last = 888;
	}
	else if ( transalg_template_cnf.file_name.find( "grain" ) != std::string::npos ) {
		generator_type = "grain";
		transalg_reg_variable_first = 1;
		transalg_reg_variable_last = 160;
		transalg_keystream_variable_first = 1786;
		transalg_keystream_variable_last = 1945;
		GoS_reg1_variable_first = 1;
		GoS_reg1_variable_last = 80;
		GoS_reg2_variable_first = 401;
		GoS_reg2_variable_last = 480;
	}
	
	std::cout << "generator_type " << generator_type << std::endl;

	std::vector<int> transalg_reg_variables;
	for ( unsigned i=transalg_reg_variable_first; i <= transalg_reg_variable_last; i++ )
		transalg_reg_variables.push_back(i);
	
	std::cout << "transalg_reg_variables " << std::endl;
	for ( unsigned i=0; i < transalg_reg_variables.size(); i++ )
		std:: cout << transalg_reg_variables[i] << " ";
	std::cout << std::endl;

	unsigned known_bits = atoi( argv[2] );
	std::vector<cnf_data> GoS_cnf_vec;
	
	std::vector<std::string> original_GoS_cnf_files_vec;
	std::vector<std::string> GoS_files;
	getdir( GoS_path, GoS_files );
	std::vector< std::vector<bool> > reg_values_vec, keystream_values_vec;
	std::cout << "GoS_files.size() " << GoS_files.size() << std::endl;
	reg_values_vec.resize( GoS_files.size() );
	keystream_values_vec.resize( reg_values_vec.size() );
	original_GoS_cnf_files_vec.resize( reg_values_vec.size() );
	std::cout << "reg_values_vec.size() " << reg_values_vec.size() << std::endl;
	
	std::vector<int> transalg_keystream_variables;
	for ( unsigned i=transalg_keystream_variable_first; i <= transalg_keystream_variable_last; i++ )
		transalg_keystream_variables.push_back(i);

	std::cout << "transalg_keystream_variables " << std::endl;
	for ( unsigned i=0; i < transalg_keystream_variables.size(); i++ )
		std:: cout << transalg_keystream_variables[i] << " ";
	std::cout << std::endl;
	
	std::vector<int> GoS_reg_variables;
	for ( unsigned i=GoS_reg1_variable_last; i >= GoS_reg1_variable_first; i-- )
		GoS_reg_variables.push_back(i);
	for ( unsigned i=GoS_reg2_variable_last; i >= GoS_reg2_variable_first; i-- )
		GoS_reg_variables.push_back(i);
	if ( generator_type == "trivium" )
		for ( unsigned i=GoS_reg3_variable_last; i >= GoS_reg3_variable_first; i-- )
			GoS_reg_variables.push_back(i);
	
	std::cout << "GoS_reg_variables " << std::endl;
	for ( unsigned i=0; i < GoS_reg_variables.size(); i++ )
		std:: cout << GoS_reg_variables[i] << " ";
	std::cout << std::endl;

	// get date from CNF: vars count, clauses count, clauses and comments
	get_cnf_data( transalg_template_cnf );

	int num;
	std::size_t found1, found2;
	std::string str, original_GoS_file_name;
	std::ofstream new_GoS_file;
	std::string str_num;
	int pos = 0;
	instances_count = 0;
	std::cout << "instances:" << std::endl;
	for ( std::vector<std::string>::iterator file_it = GoS_files.begin(); file_it != GoS_files.end(); file_it++ ) {
		if ( (*file_it).find( generator_type ) == std::string::npos )
			continue;
			
		found1 = (*file_it).find( ".cnf" );
		found2 = (*file_it).find( ".output" );
		if ( found1 != std::string::npos ) {
			std::cout << *file_it << std::endl;
			instances_count++;
			str_num = (*file_it).substr( found1-1, 1 );
			pos = found1-2;
			while ( (*file_it)[pos] != '-' ) {
				str_num = (*file_it)[pos] + str_num;
				pos--;
			}
			std::istringstream( str_num ) >> num;
			keystream_values_vec[num-1] = get_keystream_values_from_file( *file_it );
			// save to file with short name
			original_GoS_file_name = GoS_path + "/" + (*file_it);
			original_GoS_cnf_files_vec[num-1] = original_GoS_file_name;
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
	std::cout << "instances_count " << instances_count << std::endl;
	
	reg_values_vec.resize( instances_count );
	keystream_values_vec.resize( instances_count );
	original_GoS_cnf_files_vec.resize( instances_count );
	
	unsigned cnf_instance_clauses_count = transalg_template_cnf.clauses_count; 
	cnf_instance_clauses_count += transalg_keystream_variables.size();
	cnf_instance_clauses_count += known_bits;

	if ( keystream_values_vec[0].size() != transalg_keystream_variables.size() ) {
		std::cerr << "keystream_values.size() != result_keystream_variables.size()" << std::endl;
		std::cerr << keystream_values_vec[0].size() << " != " << transalg_keystream_variables.size() << std::endl;
	}
	
	std::string transalg_out_file_name, info_file_name, new_GoS_file_name;
	std::stringstream str_num_sstream, known_bit_sstream;
	std::ofstream transalg_out_file, info_file;
	known_bit_sstream << known_bits;
	
	unsigned k;
	for ( unsigned i=0; i < keystream_values_vec.size(); i++ ) {
		str_num_sstream << i;
		
		transalg_out_file_name = result_path + "/" + generator_type + "_Transalg_test" + str_num_sstream.str();
		transalg_out_file_name += "_last";
		transalg_out_file_name += known_bit_sstream.str();
		transalg_out_file_name += "known";
		transalg_out_file_name += ".cnf";
		info_file_name = result_path + "/" + generator_type + "_test" + str_num_sstream.str();
		info_file_name += ".info";
		
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
		transalg_out_file << "p cnf " << transalg_template_cnf.vars_count << " " << cnf_instance_clauses_count << std::endl;
		transalg_out_file << transalg_template_cnf.header_comments_sstream.str();
		for ( unsigned t1 = 0; t1 < known_bits; t1++ ) {
			convert_sstream << (reg_values_vec[i][t1] ? "" : "-") << transalg_reg_variables[t1] << " 0";
			transalg_out_file << convert_sstream.str() << std::endl;
			convert_sstream.str(""); convert_sstream.clear();
		}
		k = 0;
		for ( std::vector<bool>::iterator vb_it = keystream_values_vec[i].begin(); vb_it != keystream_values_vec[i].end(); vb_it++ ) {
			convert_sstream << (*vb_it ? "" : "-") << transalg_keystream_variables[k++] << " 0";
			transalg_out_file << convert_sstream.str() << std::endl;
			convert_sstream.str(""); convert_sstream.clear();
		}
		transalg_out_file << transalg_template_cnf.clauses_sstream.str();
		transalg_out_file.close(); transalg_out_file.clear();
		info_file.close(); info_file.clear();
		
		// make CNF in GoS format
		cnf_data cur_GoS_cnf;
		cur_GoS_cnf.file_name = original_GoS_cnf_files_vec[i];
		get_cnf_data( cur_GoS_cnf );
		convert_sstream << i;
		new_GoS_file_name = result_path + "/" + generator_type + "_GoS_test" + convert_sstream.str();
		convert_sstream.str(""); convert_sstream.clear();
		new_GoS_file_name += "_last"; 
		new_GoS_file_name += known_bit_sstream.str();
		new_GoS_file_name += "known";
		new_GoS_file_name += ".cnf";
		new_GoS_file.open( new_GoS_file_name.c_str() );
		new_GoS_file << "p cnf " << cur_GoS_cnf.vars_count << " " << cur_GoS_cnf.clauses_count + known_bits << std::endl;
		new_GoS_file << cur_GoS_cnf.header_comments_sstream.str();
		
		for ( unsigned t1 = 0; t1 < known_bits; t1++ ) {
			convert_sstream << (reg_values_vec[i][t1] ? "" : "-") << GoS_reg_variables[t1] << " 0";
			new_GoS_file << convert_sstream.str() << std::endl;
			convert_sstream.str(""); convert_sstream.clear();
		}
		
		new_GoS_file << cur_GoS_cnf.clauses_sstream.str();
		new_GoS_file.close(); new_GoS_file.clear();
	}
	
	return 0;
}

std::vector<bool> get_keystream_values_from_file( std::string keystream_file_name )
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
	unsigned reg1_len = 0, reg2_len = 0, reg3_len = -1000;
	if ( generator_type == "bivium" ) {
		reg1_len = 93;
		reg2_len = 84;
	}
	else if ( generator_type == "grain" ) {
		reg1_len = 80;
		reg2_len = 80;
	}
	else if ( generator_type == "trivium" ) {
		reg1_len = 93;
		reg2_len = 84;
		reg3_len = 111;
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
		if ( reg_values.size() == reg1_len + reg2_len + reg3_len ) {
			tmp_reg_values = reg_values;
			reg_values.resize(reg1_len + reg2_len);
			unsigned k=0;
			for ( std::vector<bool>::reverse_iterator r_it = tmp_reg_values.rbegin(); r_it != tmp_reg_values.rend(); r_it++ ) {
				reg_values.push_back(*r_it);
				k++;
				if ( k == reg3_len  )
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

void get_cnf_data( cnf_data &cnf ) 
{
	std::ifstream cnf_file( cnf.file_name.c_str() );
	if ( !cnf_file.is_open() ) {
		std:: cerr << "!template_file.is_open()" << std::endl;
		exit(1);
	}
	
	cnf.vars_count = cnf.clauses_count = 0;
	bool isClausesStrings = false;
	bool isFirstClauseString = true;
	std::stringstream str_sstream;
	int val;
	std::string str;
	while ( getline( cnf_file, str ) ) {
		str.erase( std::remove(str.begin(), str.end(), '\r'), str.end() );
		if ( str[0] == 'p' )
			continue;
		if ( ( str[0] == 'c' ) && ( !isClausesStrings ) )
			cnf.header_comments_sstream << str << std::endl;
		else {
			if ( !isFirstClauseString )
				cnf.clauses_sstream << std::endl;
			cnf.clauses_sstream << str;
			isClausesStrings = true;
			isFirstClauseString = false;
			if ( str[0] != 'c' )
				cnf.clauses_count++;
			str_sstream << str;
			while ( str_sstream >> val )
				if ( abs(val) > (int)cnf.vars_count )
					cnf.vars_count = abs(val);
			str_sstream.clear(); str_sstream.str("");
		}
	}
	cnf_file.close();
}