#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "minisat22_wrapper.h"

int main( int argc, char *argv[] )
{
	std::vector<std::vector<unsigned>> vectors_vec, vectors_vec2;
	std::stringstream sstream;
	std::string str, vectors_file_name;
	
#ifdef _DEBUG
	argc = 3;
	argv[1] = "vectors";
	argv[2] = "MD4_round_48.cnf";
#endif

	if ( argc != 3 ) {
		std::cerr << "Usage : vectrors CNF";
		return 1;
	}
	
	vectors_file_name = argv[1];
	std::ifstream ifile(vectors_file_name.c_str());
	std::vector<unsigned> cur_vector;
	unsigned uval;
	while ( getline(ifile, str) ) {
		sstream << str;
		while (sstream >> uval)
			cur_vector.push_back(uval);
		sstream.str("");
		sstream.clear();
		vectors_vec.push_back(cur_vector);
		cur_vector.clear();
	}
	ifile.close();

	/*for (unsigned i=0; i < vectors_vec.size(); i++) {
		if ((i > 0) && (i % 32 == 0)) {
			vectors_vec2.push_back(cur_vector);
			cur_vector.clear();
		}
		cur_vector.push_back(vectors_vec[i][0]);
	}
	vectors_vec2.push_back(cur_vector);

	std::ofstream ofile("out");
	for(auto &x : vectors_vec2) {
		for (auto &y : x)
			ofile << y << " ";
		ofile << std::endl;
	}
	ofile.close();*/
	
	unsigned bool_values_number = 1 << vectors_vec.size();
	unsigned cur_index;
	std::vector<bool> cur_bool_value_vec;
	std::vector<std::vector<bool>> bool_values_vec;
	cur_bool_value_vec.resize(vectors_vec.size());
	unsigned k = 0;
	for ( unsigned i=0; i < bool_values_number; i++) {
		for (auto &x : cur_bool_value_vec)
			x = false;
		cur_index = i;
		k = 0;
		while (cur_index > 0) {
			cur_bool_value_vec[cur_bool_value_vec.size() - k - 1] = cur_index % 2 == 1 ? true : false;
			cur_index /= 2;
			k++;
		}
		bool_values_vec.push_back(cur_bool_value_vec);
	}
	
	vec< vec<Lit> > dummy_vec;
	dummy_vec.resize(bool_values_vec.size());
	for (unsigned i = 0; i < bool_values_vec.size(); i++)
		for (unsigned j = 0; j < bool_values_vec[i].size(); j++)
			if (bool_values_vec[i][j])
				for (unsigned j2 = 0; j2 < vectors_vec[j].size(); j2++)
					dummy_vec[i].push(~mkLit(vectors_vec[j][j2] - 1));
	
	std::string input_cnf_name = argv[2];
	std::ifstream in;
	minisat22_wrapper m22_wrapper;
	Solver *S;
	in.open(input_cnf_name); // read every new batch because CNF can be changed
	Problem cnf;
	m22_wrapper.parse_DIMACS_to_problem(in, cnf);
	in.close(); in.clear();
	S = new Solver();
	S->addProblem(cnf);
	S->verbosity = 0;
	S->isPredict = false;
	S->max_nof_restarts = 1;

	in.open(input_cnf_name);
	std::stringstream cnf_sstream;
	while (getline(in, str))
		cnf_sstream << str << std::endl;
	in.close();
	std::string cnf_addit_cond_name;
	std::ofstream cnf_addit_cond;
	
	// add values of the output variables
	/*int out_first_var = 8630, out_last_var = 8757;
	for (int i = out_first_var; i <= out_last_var; i++)
		S->addClause(~mkLit(i-1));*/
	
	unsigned add_cnf_count = 0;
	lbool ret;
	unsigned interrupted = 0, unsat = 0, sat = 0;
	std::cout << "interrupted values :" << std::endl;
	unsigned weight;
	for (int i = 0; i < dummy_vec.size(); i++) {
		ret = S->solveLimited(dummy_vec[i]);
		if (ret == l_Undef)
			interrupted++;
		else if (ret == l_False)
			unsat++;
		else if (ret == l_True)
			sat++;
		weight = 0;
		for (auto &x : bool_values_vec[i])
			if (x == true) weight++;
		std::cout << i << " : " << weight << " ";
		if (ret == l_Undef) {
			std::cout << "UNDEF ";
			if (weight == 14) {
				add_cnf_count++;
				sstream << input_cnf_name << "_add_" << add_cnf_count;
				cnf_addit_cond_name = sstream.str();
				cnf_addit_cond.open(cnf_addit_cond_name.c_str());
				cnf_addit_cond << cnf_sstream.str();
				sstream.str(""); sstream.clear();
				for (unsigned j = 0; j < bool_values_vec[i].size(); j++)
					if (bool_values_vec[i][j])
						for (unsigned j2 = 0; j2 < vectors_vec[j].size(); j2++)
							cnf_addit_cond << "-" << vectors_vec[j][j2] << " 0" << std::endl;
				cnf_addit_cond.close();
				cnf_addit_cond.clear();
			}
		}
		else if (ret == l_False)
			std::cout << "UNSAT ";
		else if (ret == l_True)
			std::cout << "SAT ";
		for (auto &x : bool_values_vec[i])
			std::cout << x;
		std::cout << std::endl;
		//if (i % 1000 == 0)
		//	std::cout << "processed " << i << " values from " << dummy_vec.size() << std::endl;
	}
	
	std::cout << "interrupted " << interrupted << " from " << dummy_vec.size() << std::endl;
	std::cout << "unsat " << unsat << " from " << dummy_vec.size() << std::endl;
	std::cout << "sat " << sat << " from " << dummy_vec.size() << std::endl;
	
	delete S;
	
	return 0;
}