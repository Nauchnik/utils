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
	in.close();
	S = new Solver();
	S->addProblem(cnf);
	S->verbosity = 0;
	S->isPredict = false;
	S->max_nof_restarts = 2;

	lbool ret;
	unsigned interrupted = 0;
	std::cout << "interrupted values :" << std::endl;
	unsigned weight = 0;
	for (int i = 0; i < dummy_vec.size(); i++) {
		ret = S->solveLimited(dummy_vec[i]);
		if (ret == l_Undef) {
			interrupted++;
			weight = 0;
			for (auto &x : bool_values_vec[i])
				if (x == true ) weight++;
			std::cout << weight << " : ";
			for (auto &x : bool_values_vec[i])
				std::cout << x;
			std::cout << std::endl;
		}
		//if (i % 1000 == 0)
		//	std::cout << "processed " << i << " values from " << dummy_vec.size() << std::endl;
	}
	
	std::cout << "interrupted " << interrupted << " from " << dummy_vec.size() << std::endl;
	
	delete S;
	
	return 0;
}