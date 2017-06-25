#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
#ifdef _DEBUG
	argc = 3;
	argv[1] = "testing_sat_solvers_out";
	argv[2] = "out.csv";
#endif
	string file_name = argv[1];
	string ofile_name = argv[2];

	vector<vector<double>> values_vec_vec;
	vector<string> solvers_names;
	vector<string> cnfs_names;
	string str, cur_solver_name, cur_cnf_name;
	stringstream sstream;
	
	ifstream ifile(file_name);
	if (ifile.is_open()) {
		while (getline(ifile,str)) {
			sstream.str("");
			sstream.clear();
			if (str == "") 
				continue;
			sstream << str;
			sstream >> cur_solver_name;
			if (cur_solver_name == "instance")
				continue;
			if (std::find(solvers_names.begin(), solvers_names.end(), cur_solver_name) == solvers_names.end())
				solvers_names.push_back(cur_solver_name);
			sstream >> cur_cnf_name;
			if (std::find(cnfs_names.begin(), cnfs_names.end(), cur_cnf_name) == cnfs_names.end())
				cnfs_names.push_back(cur_cnf_name);
			unsigned solver_index, cnf_index;
			for (unsigned j = 0; j < solvers_names.size(); j++)
				if (cur_solver_name == solvers_names[j]) {
					solver_index = j;
					break;
				}
			for (unsigned j = 0; j < cnfs_names.size(); j++)
				if (cur_cnf_name == cnfs_names[j]) {
					cnf_index = j;
					break;
				}
			double dval;
			sstream >> dval; // skip result status
			sstream >> dval;
			if (values_vec_vec.size() < cnf_index + 1)
				values_vec_vec.resize(cnf_index + 1);
			if (values_vec_vec[cnf_index].size() < solver_index + 1)
				values_vec_vec[cnf_index].resize(solver_index + 1);
			values_vec_vec[cnf_index][solver_index] = dval;
		}
	}

	std::string head_str = "Instance";
	for (auto &x : solvers_names)
		head_str += " " + x;
	std::ofstream ofile(ofile_name);
	ofile << head_str << std::endl;
	for (unsigned i=0; i < values_vec_vec.size(); i++) {
		ofile << cnfs_names[i];
		for (unsigned j = 0; j < values_vec_vec[i].size(); j++)
			ofile << " " << values_vec_vec[i][j];
		ofile << std::endl;
	}
	ofile.close();

	return 0;
}