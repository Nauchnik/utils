#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

int main( int argc, char** argv )
{
	string decomposition_file_name, assumptions_file_name, dnf_file_name;
	ifstream decomposition_file, assumptions_file;
	ofstream dnf_file;
	stringstream sstream;

#ifdef _DEBUG
	argc = 4;
	argv[1] = "known_point";
	argv[2] = "assumptions2.txt";
	argv[3] = "dnf_file.txt";
#endif
	
	if ( argc < 4 ) {
		cerr << "Usage: decomposition_file assumptions_file dnf_file";
		exit;
	}
	
	decomposition_file_name = argv[1];
	assumptions_file_name   = argv[2];
	dnf_file_name           = argv[3];
	
	decomposition_file.open( decomposition_file_name );
	string str;
	getline( decomposition_file, str );
	decomposition_file.close();
	sstream << str;
	int var;
	vector<int> var_vec, activity_order;
	unsigned clauses_count = 0, literals_count = 0, min_var_num, max_var_num;
	
	while ( sstream >> var ) {
		if ( var_vec.size() == 0 )
			min_var_num = max_var_num = var; // 1st time
		else if ( var < min_var_num )
			min_var_num = var;
		else if ( var > max_var_num )
			max_var_num = var;
		var_vec.push_back( var );
		activity_order.push_back( abs( var ) );
	}
	
	stringstream sstream_out;
	assumptions_file.open( assumptions_file_name );
	while ( getline( assumptions_file, str ) ) {
		if ( !str.size() ) continue;
		if ( str.size() != var_vec.size() ) {
			cerr << "str.size() != var_vec.size()";
			exit;
		}
		for ( unsigned i=0; i < str.size(); ++i ) {
			if ( str[i] == '0' )
				sstream_out << '-';
			sstream_out << var_vec[i] << " ";
		}
		sstream_out << "0" << endl;
		clauses_count++;
		if ( clauses_count % 10000 == 0 )
			cout << clauses_count << endl;
	}
	assumptions_file.close();
	literals_count = clauses_count * var_vec.size();

	dnf_file.open( dnf_file_name, ios_base :: out );
	dnf_file << "p dnf " << var_vec.size() << " " << clauses_count << endl;
	dnf_file << "c literals_count " << literals_count << endl;
	dnf_file << "c min_var_num " << min_var_num << endl;
	dnf_file << "c max_var_num " << max_var_num << endl;
	dnf_file << "c activity_order ";
	for ( unsigned i=0; i < activity_order.size(); ++i )
		dnf_file << activity_order[i] << " ";
	dnf_file << "0" << endl;
	dnf_file << sstream_out.rdbuf();
	dnf_file.close();

	return 0;
}