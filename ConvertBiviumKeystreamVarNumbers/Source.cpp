#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

int main( int argc, char **argv ) 
{
#ifdef _DEBUG
	argc = 3;
	argv[1] = "bivium_template_new.cnf";
	argv[2] = "bivium_stream_0";
#endif
	std::string template_file_name;
	std::string keystream_file_name;
	if ( argc != 3 ) {
		std::cerr << "Usage: [CNF template file] [Bivium keystream file]";
		return 1;
	}
	
	template_file_name = argv[1];
	keystream_file_name = argv[2];
	std::ifstream template_file( template_file_name.c_str() );
	std::stringstream template_sstream;
	std::string str;
	int variables_count = 0;
	std::stringstream convert_sstream;
	std::string str1;
	while ( getline( template_file, str ) ) {
		template_sstream << str << std::endl;
		if ( str.find( "p cnf " ) != std::string::npos ) {
			convert_sstream << str;
			convert_sstream >> str1 >> str1 >> variables_count;
			std::cout << "variables_count " << variables_count << std::endl;
			convert_sstream.str("");
			convert_sstream.clear();
		}
	}
	template_file.close();

	int val;
	std::vector<bool> keystream_values;
	std::ifstream keystream_file( keystream_file_name.c_str() );
	while ( keystream_file >> val )
		keystream_values.push_back( (val > 0) ? true : false );
	keystream_file.close();

	std::string out_file_name = "out.cnf";
	std::ofstream out_file( out_file_name.c_str() );
	out_file << template_sstream.rdbuf();
	for ( unsigned i = 0; i < keystream_values.size(); i++ ) {
		convert_sstream << (keystream_values[i] ? "" : "-") << variables_count + i + 1 - keystream_values.size() << " 0";
		out_file << convert_sstream.str();
		if ( i != keystream_values.size()-1 )
			out_file << std::endl;
		convert_sstream.str(""); convert_sstream.clear();
	}
	
	out_file.close();

	return 0;
}