#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include "D:/Programming/DiagonalLatinSquaresGenerator/odls_sequential.h"

void constructPseudotripleCNFs(std::string pseudotriple_template_cnf_name, 
	                           unsigned characteristics_from, unsigned characteristics_to, 
							   bool isPairsUsing, std::string known_podls_file_name);
bool checkPlingelingSolution(std::vector<odls_pair> odls_pair_vec );

int main(int argc, char **argv)
{
#ifdef _DEBUG
	argc = 5;
	argv[1] = "pseudotriple_dls_10_template.cnf";
	argv[2] = "D:/Programming/pdsat/src_common/ODLS_10_pairs.txt";
	argv[3] = "60";
	argv[4] = "62";
	//argv[5] = "-no_pairs";
#endif

	if ( (argc < 5 ) || (argc > 6) ) {
		std::cerr << "Usage : pseudotriple_template_cnf_name known_podls_file_name characteristics_from characteristics_to [-no_pairs]";
		return 1;
	}

	std::string pseudotriple_template_cnf_name = argv[1];
	std::string known_podls_file_name = argv[2];
	unsigned characteristics_from = atoi(argv[3]);
	unsigned characteristics_to = atoi(argv[4]);
	std::string no_pairs_str;
	bool isPairsUsing = true;
	if (argc > 5) {
		no_pairs_str = argv[5];
		if (no_pairs_str == "-no_pairs")
			isPairsUsing = false;
	}
	
	std::cout << "pseudotriple_template_cnf_name " << pseudotriple_template_cnf_name << std::endl;
	std::cout << "known_podls_file_name " << known_podls_file_name << std::endl;
	std::cout << "characteristics_from " << characteristics_from << std::endl;
	std::cout << "characteristics_to " << characteristics_to << std::endl;
	std::cout << "isPairsUsing " << isPairsUsing << std::endl;
	
	constructPseudotripleCNFs(pseudotriple_template_cnf_name, characteristics_from, 
		characteristics_to, isPairsUsing, known_podls_file_name);
	
	return 0;
}

void constructPseudotripleCNFs(std::string pseudotriple_template_cnf_name, 
							   unsigned characteristics_from, 
							   unsigned characteristics_to, 
							   bool isPairsUsing,
							   std::string known_podls_file_name)
{
	// creating SAT encodings mode, instead of pure DLS generating mode
	std::stringstream dls_pair_clauses_sstream, template_clauses_sstream, cells_restr_clause_sstream, tmp_sstream;
	std::cout << "pseudotriple_template_cnf_name " << pseudotriple_template_cnf_name << std::endl;
	std::ifstream ifile(pseudotriple_template_cnf_name.c_str());
	if (!ifile.is_open()) {
		std::cerr << pseudotriple_template_cnf_name << " not open" << std::endl;
		exit(1);
	}
	std::vector<unsigned> cells_restr_var_numbers;
	unsigned uval;
	std::string str;
	while (std::getline(ifile, str)) {
		tmp_sstream.str(""); tmp_sstream.clear();
		tmp_sstream << str;
		str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
		template_clauses_sstream << str << std::endl;
		if (cells_restr_var_numbers.size() == 100)
			continue;
		else
			cells_restr_var_numbers.clear();
		while (tmp_sstream >> uval) {
			if (uval)
				cells_restr_var_numbers.push_back(uval);
		}
	}
	ifile.close();
	if (cells_restr_var_numbers.size() != 100) {
		std::cerr << "cells_restr_var_numbers.size() != 100 ";
		exit(1);
	}

	odls_sequential odls_seq;
	odls_seq.readOdlsPairs(known_podls_file_name);
	std::string cur_pseudotriple_file_name;
	std::ofstream cur_pseudotriple_file;
	unsigned pair_index = 0;
	std::string system_str;
	unsigned dls_index = 0, row_index = 0, column_index = 0;

	checkPlingelingSolution(odls_seq.odls_pair_vec);

	if (isPairsUsing) {
		for (auto &x : odls_seq.odls_pair_vec) { // for every pair of dls make cnf for searching pseudotriple
			// write 1st known DLS in the form of oneliteral clauses (1 clause for each DLS cell)
			dls_index = 0;
			for (row_index = 0; row_index < x.dls_1.size(); row_index++)
				for (column_index = 0; column_index < x.dls_1[row_index].size(); column_index++)
					dls_pair_clauses_sstream << 1000 * dls_index + 100 * row_index + 10 * column_index + (x.dls_1[row_index][column_index] - 48) + 1 << " 0\n";
			// write 2nd known DLS in the form of oneliteral clauses (1 clause for each DLS cell)
			dls_index = 1;
			for (row_index = 0; row_index < x.dls_2.size(); row_index++)
				for (column_index = 0; column_index < x.dls_2[row_index].size(); column_index++)
					dls_pair_clauses_sstream << 1000 * dls_index + 100 * row_index + 10 * column_index + (x.dls_2[row_index][column_index] - 48) + 1 << " 0\n";
			
			// write "0 1 2 3 4 5 6 7 8 9" as first row of the 3rd DLS
			dls_index = 2;
			row_index = 0;
			for (column_index = 0; column_index < 10; column_index++) {
				dls_pair_clauses_sstream << 1000 * dls_index + 100 * row_index + 10 * column_index + column_index + 1 << " 0";
				if ( column_index < 10)
					dls_pair_clauses_sstream << "\n";
				else
					dls_pair_clauses_sstream << " "; // for treengling
			}
			
			for (unsigned i = characteristics_from; i <= characteristics_to; i++) {
				cur_pseudotriple_file_name = "dls-pseudotriple_";
				tmp_sstream.clear(); tmp_sstream.str("");
				tmp_sstream << i;
				cur_pseudotriple_file_name += tmp_sstream.str();
				tmp_sstream.clear(); tmp_sstream.str("");
				cur_pseudotriple_file_name += "cells_pair";
				tmp_sstream << pair_index;
				cur_pseudotriple_file_name += tmp_sstream.str();
				cur_pseudotriple_file_name += ".cnf";
				cells_restr_clause_sstream << cells_restr_var_numbers[i - 1] << " 0\n";
				cur_pseudotriple_file.open(cur_pseudotriple_file_name.c_str(), std::ios_base::out);
				cur_pseudotriple_file << template_clauses_sstream.str();
				cur_pseudotriple_file << cells_restr_clause_sstream.str();
				cur_pseudotriple_file << dls_pair_clauses_sstream.str();
				cur_pseudotriple_file.close();
				cells_restr_clause_sstream.str(""); cells_restr_clause_sstream.clear();
				system_str = "./fix_cnf " + cur_pseudotriple_file_name;
				std::cout << "system_str " << system_str << std::endl;
				system(system_str.c_str());
			}
			dls_pair_clauses_sstream.str(""); dls_pair_clauses_sstream.clear();
			pair_index++;
		}
	}
	else {
		// using no pairs
		for (unsigned i = characteristics_from; i <= characteristics_to; i++) {
			cur_pseudotriple_file_name = "dls-pseudotriple_";
			tmp_sstream.clear(); tmp_sstream.str("");
			tmp_sstream << i;
			cur_pseudotriple_file_name += tmp_sstream.str();
			tmp_sstream.clear(); tmp_sstream.str("");
			cur_pseudotriple_file_name += ".cnf";
			cells_restr_clause_sstream << cells_restr_var_numbers[i - 1] << " 0\n";
			cur_pseudotriple_file.open(cur_pseudotriple_file_name.c_str(), std::ios_base::out);
			cur_pseudotriple_file << template_clauses_sstream.str();
			cur_pseudotriple_file << cells_restr_clause_sstream.str();
			cur_pseudotriple_file << dls_pair_clauses_sstream.str();
			cur_pseudotriple_file.close();
			cells_restr_clause_sstream.str(""); cells_restr_clause_sstream.clear();
			system_str = "./fix_cnf " + cur_pseudotriple_file_name;
			std::cout << "system_str " << system_str << std::endl;
			system(system_str.c_str());
		}
	}
}

bool checkPlingelingSolution( std::vector<odls_pair> odls_pair_vec )
{
	// check solution
	std::stringstream sstream;
	//ReadOdlsPairs( odls_pair_vec );
	std::string solutionfile_name = "out_treengeling_dls-pseudotriple_73cells_pair1.cnf";
	std::ifstream solutionfile( solutionfile_name.c_str(), std::ios_base::in );
	std::string str;
	dls new_dls1, new_dls2, new_dls3;
	odls_pair cur_pair;
	std::string dls_row1, dls_row2, dls_row3;
	int val;
	
	if ( !solutionfile.is_open() ) {
		std::cerr << "solutionfile " << solutionfile_name << " not open" << std::endl;
		return false;
	}

	while ( std::getline( solutionfile, str ) ) {
		if ( ( str[0] == 'v' ) && ( str[1] == ' ' ) ) {
			sstream << str.substr(2);
			while ( sstream >> val ) {
				if ((val >= 1) && (val <= 1000)) {
					val = val % 10 ? (val % 10) - 1 : 9;
					dls_row1.push_back('0' + val);
				}
				if ((val >= 1001) && (val <= 2000)) {
					val = val % 10 ? (val % 10) - 1 : 9;
					dls_row2.push_back('0' + val);
				}
				if ( ( val >= 2001 ) && ( val <= 3000 ) ) { // for 3rd DLS
					val = val % 10 ? (val % 10) - 1 : 9;
					dls_row3.push_back( '0' + val );
				}
				if (dls_row1.size() == 10) {
					new_dls1.push_back(dls_row1);
					std::cout << dls_row1 << std::endl;
					dls_row1 = "";
				}
				if (dls_row2.size() == 10) {
					new_dls2.push_back(dls_row2);
					std::cout << dls_row2 << std::endl;
					dls_row2 = "";
				}
				if ( dls_row3.size() == 10 ) {
					new_dls3.push_back( dls_row3 );
					std::cout << dls_row3 << std::endl;
					dls_row3 = "";
				}
			}
			sstream.clear(); sstream.str("");
		}
	}
	solutionfile.close();
	std::cout << std::endl;
	for ( auto &x :new_dls3){
		for ( unsigned i=0;i<x.size(); i++ )
			std::cout << x[i] << " ";
		std::cout << std::endl;
	}
	cur_pair.dls_1 = new_dls1;
	cur_pair.dls_2 = new_dls2;
	odls_pseudotriple pseudotriple;
	odls_sequential odls_seq;
	odls_seq.makePseudotriple(cur_pair, new_dls3, pseudotriple);
	for (auto &x : pseudotriple.unique_orthogonal_cells)
		std::cout << x << " ";

	// check Brown pseudotriple
	std::cout << std::endl;
	odls_seq.makePseudotriple(odls_pair_vec[17], odls_pair_vec[18].dls_1, pseudotriple);
	std::cout << "Brown pseudotriple.unique_orthogonal_cells.size() " << pseudotriple.unique_orthogonal_cells.size() << std::endl;
	for (auto &x : pseudotriple.unique_orthogonal_cells)
		std::cout << x << " ";
	// 1st new pseudotriple
	odls_seq.makePseudotriple( odls_pair_vec[17], new_dls3, pseudotriple );
	std::cout << "new pseudotriple 1 pseudotriple.unique_orthogonal_cells.size() " << pseudotriple.unique_orthogonal_cells.size() << std::endl;
	for ( auto &x : pseudotriple.unique_orthogonal_cells )
		std::cout << x << " ";
	odls_seq.makePseudotriple(odls_pair_vec[18], new_dls3, pseudotriple);
	std::cout << "new pseudotriple 2 pseudotriple.unique_orthogonal_cells.size() " << pseudotriple.unique_orthogonal_cells.size() << std::endl;
	for (auto &x : pseudotriple.unique_orthogonal_cells)
		std::cout << x << " ";
	// 3 new pseudotriples based of 4th DLS orthogonal with Brawn DLS
	odls_seq.makePseudotriple(odls_pair_vec[17], odls_pair_vec[42].dls_2, pseudotriple);
	std::cout << "new pseudotriple 3 pseudotriple.unique_orthogonal_cells.size() " << pseudotriple.unique_orthogonal_cells.size() << std::endl;
	for (auto &x : pseudotriple.unique_orthogonal_cells)
		std::cout << x << " ";
	odls_seq.makePseudotriple(odls_pair_vec[18], odls_pair_vec[42].dls_2, pseudotriple);
	std::cout << "new pseudotriple 4 pseudotriple.unique_orthogonal_cells.size() " << pseudotriple.unique_orthogonal_cells.size() << std::endl;
	for (auto &x : pseudotriple.unique_orthogonal_cells)
		std::cout << x << " ";
	odls_seq.makePseudotriple(odls_pair_vec[41], odls_pair_vec[42].dls_2, pseudotriple);
	std::cout << "new pseudotriple 5 pseudotriple.unique_orthogonal_cells.size() " << pseudotriple.unique_orthogonal_cells.size() << std::endl;
	for (auto &x : pseudotriple.unique_orthogonal_cells)
		std::cout << x << " ";

	return true;
}
