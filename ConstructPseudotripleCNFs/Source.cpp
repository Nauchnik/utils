#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include "../../DiagonalLatinSquaresGenerator/odls_sequential.h"

void constructPseudotripleCNFs(std::string pseudotriple_template_cnf_name, unsigned characteristics_from, unsigned characteristics_to, bool isPairsUsing);

int main(int argc, char **argv)
{
#ifdef _DEBUG
	argc = 2;
	argv[1] = "pseudotriple_dls_10_template.cnf";
	argv[2] = "-no_pairs";
#endif

	if (argc > 5) {
		std::cerr << "Usage : pseudotriple_template_cnf_name characteristics_from characteristics_to [-no_pairs]";
		return 1;
	}

	std::string pseudotriple_template_cnf_name = argv[1];
	unsigned characteristics_from = atoi(argv[2]);
	unsigned characteristics_to = atoi(argv[3]);
	std::string no_pairs_str;
	bool isPairsUsing = true;
	if (argc > 4) {
		no_pairs_str = argv[4];
		if (no_pairs_str == "-no_pairs")
			isPairsUsing = false;
	}
	
	std::cout << "pseudotriple_template_cnf_name " << pseudotriple_template_cnf_name << std::endl;
	std::cout << "characteristics_from " << characteristics_from << std::endl;
	std::cout << "characteristics_to " << characteristics_to << std::endl;
	std::cout << "isPairsUsing " << isPairsUsing << std::endl;
	
	constructPseudotripleCNFs(pseudotriple_template_cnf_name, characteristics_from, characteristics_to, isPairsUsing );
	
	return 0;
}

void constructPseudotripleCNFs(std::string pseudotriple_template_cnf_name, unsigned characteristics_from, unsigned characteristics_to, bool isPairsUsing)
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
	std::vector<odls_pair> odls_pair_vec;
	odls_seq.readOdlsPairs(odls_pair_vec);
	std::string cur_pseudotriple_file_name;
	std::ofstream cur_pseudotriple_file;
	unsigned pair_index = 0;
	std::string system_str;

	if (isPairsUsing) {
		for (auto &x : odls_pair_vec) { // for every pair of dls make cnf for searching pseudotriple
			for (unsigned i = 0; i < x.dls_1.size(); i++)
				for (unsigned j = 0; j < x.dls_1[i].size(); j++)
					dls_pair_clauses_sstream << 100 * i + 10 * j + (x.dls_1[i][j] - 48) + 1 << " 0\n"; // char to int
			for (unsigned i = 0; i < x.dls_2.size(); i++)
				for (unsigned j = 0; j < x.dls_2[i].size(); j++) {
					dls_pair_clauses_sstream << 1 * 1000 + 100 * i + 10 * j + (x.dls_2[i][j] - 48) + 1 << " 0";
					if ((i == x.dls_2.size() - 1) && (j == x.dls_2[i].size() - 1))
						dls_pair_clauses_sstream << " "; // for treengling
					else
						dls_pair_clauses_sstream << "\n";
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

	/*
	// check solution
	stringstream sstream;
	ReadOdlsPairs( odls_pair_vec );
	std::string solutionfile_name = "out_treengeling_dls-pseudotriple_73cells_pair1.cnf";
	std::ifstream solutionfile( solutionfile_name.c_str(), std::ios_base::in );
	std::string str;
	dls new_dls;
	std::string dls_row;
	int val;
	if ( !solutionfile.is_open() ) {
	std::cerr << "solutionfile " << solutionfile_name << " not open" << std::endl;
	return 0;
	}

	while ( std::getline( solutionfile, str ) ) {
	if ( ( str[0] == 'v' ) && ( str[1] == ' ' ) ) {
	sstream << str.substr(2);
	while ( sstream >> val ) {
	if ( ( val >= 2001 ) && ( val <= 3000 ) ) {
	val = val % 10 ? (val % 10)-1 : 9;
	dls_row.push_back( '0' + val );
	}
	if ( dls_row.size() == 10 ) {
	new_dls.push_back( dls_row );
	std::cout << dls_row << std::endl;
	dls_row = "";
	}
	}
	sstream.clear(); sstream.str("");
	}
	}
	std::cout << std::endl;

	solutionfile.close();
	odls_pseudotriple pseudotriple;
	MakePseudotriple( odls_pair_vec[1], new_dls, pseudotriple );
	std::cout << "pseudotriple.unique_orthogonal_cells.size() " << pseudotriple.unique_orthogonal_cells.size() << std::endl;
	for ( auto &x : pseudotriple.unique_orthogonal_cells )
	std::cout << x << " ";


	// check Brown pseudotriple
	std::cout << std::endl;
	MakePseudotriple( odls_pair_vec[17], odls_pair_vec[18].dls_1, pseudotriple );
	std::cout << "Brown pseudotriple.unique_orthogonal_cells.size() " << pseudotriple.unique_orthogonal_cells.size() << std::endl;
	for ( auto &x : pseudotriple.unique_orthogonal_cells )
	std::cout << x << " ";
	*/
}
