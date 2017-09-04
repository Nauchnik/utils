#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <vector>
#include <map>

using namespace std;

const double TRESHOLD_SOLVER_TIME = 2400;
const double CLAUSES_COLLECT_TIME = 60;

vector<string> solvers_names;
vector<string> cnfs_names;
vector<string> reduced_cnfs_names;
vector<vector<double>> values_vec_vec;

string ext_solver_name = "";

bool isNumber(string str);
string getReducedCnfName(string initial_cnf_name);
double getReducedTimeValueConseqMode(const unsigned reduced_cnf_index, const unsigned solver_index);
double getReducedTimeValueParallelMode(const unsigned reduced_cnf_index, const unsigned solver_index);
void writeValuesToCsvFile(string ofile_name, vector<string> local_cnfs_names, vector<vector<double>> local_values_v_v);
void writeValuesToSimpleFile(string ofile_name, vector<string> local_cnfs_names, vector<vector<double>> local_values_v_v);

int main(int argc, char** argv)
{
#ifdef _DEBUG
	argc = 4;
	//argv[1] = "gifford_minisat+minisat";
	argv[1] = "gifford_minisat_rand";
	argv[2] = "out.csv";
	//argv[3] = "NailSAT";
	argv[3] = "minisat_rand";
#endif
	string file_name = argv[1];
	string ofile_name = argv[2];

	if (argc > 3)
		ext_solver_name = argv[3];

	string str, cur_solver_name, cur_cnf_name;
	stringstream sstream;
	
	ifstream ifile(file_name);
	if (!ifile.is_open()) {
		cerr << "input data file " << file_name << " wasn't opened" << endl;
		exit(-1);
	}
	
	while (getline(ifile,str)) {
		sstream.str("");
		sstream.clear();
		if (str == "") 
			continue;
		sstream << str;
		sstream >> cur_solver_name;
		if ( (cur_solver_name == "instance") || (cur_solver_name == "solver") )
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
		sstream >> dval;
		if (values_vec_vec.size() < cnf_index + 1)
			values_vec_vec.resize(cnf_index + 1);
		if (values_vec_vec[cnf_index].size() < solver_index + 1)
			values_vec_vec[cnf_index].resize(solver_index + 1);
		values_vec_vec[cnf_index][solver_index] = dval;
	}
	
	writeValuesToCsvFile(ofile_name, cnfs_names, values_vec_vec);

	if (ext_solver_name != "") {
		// NailSat mode (YSC'2017)

		for (auto& cnf_name : cnfs_names) {
			string new_cnf_name = getReducedCnfName(cnf_name);
			if (std::find(reduced_cnfs_names.begin(), reduced_cnfs_names.end(), new_cnf_name) == reduced_cnfs_names.end())
				reduced_cnfs_names.push_back(new_cnf_name);
		}

		vector<vector<double>> reduced_values_vec_vec_conseq;
		reduced_values_vec_vec_conseq.resize(reduced_cnfs_names.size());
		for (unsigned i = 0; i < reduced_cnfs_names.size(); i++) {
			reduced_values_vec_vec_conseq[i].resize(solvers_names.size());
			for (unsigned j = 0; j < solvers_names.size(); j++)
				reduced_values_vec_vec_conseq[i][j] = getReducedTimeValueConseqMode(i, j);
		}

		vector<vector<double>> reduced_values_vec_vec_parallel;
		reduced_values_vec_vec_parallel.resize(reduced_cnfs_names.size());
		for (unsigned i = 0; i < reduced_cnfs_names.size(); i++) {
			reduced_values_vec_vec_parallel[i].resize(solvers_names.size());
			for (unsigned j = 0; j < solvers_names.size(); j++)
				reduced_values_vec_vec_parallel[i][j] = getReducedTimeValueParallelMode(i, j);
		}

		string nail_file_name_conseq = ext_solver_name + "_conseq";
		solvers_names.resize(1);
		solvers_names[0] = ext_solver_name;
		writeValuesToSimpleFile(nail_file_name_conseq, reduced_cnfs_names, reduced_values_vec_vec_conseq);
		string nail_file_name_parallel = ext_solver_name + "_parallel";
		solvers_names[0] = ext_solver_name + "_par";
		writeValuesToSimpleFile(nail_file_name_parallel, reduced_cnfs_names, reduced_values_vec_vec_parallel);
	}
	
	return 0;
}

bool isNumber(string str)
{
	for (auto &x : str)
		if (!isdigit(x))
			return false;
	return true;
}

string getReducedCnfName(string initial_cnf_name) 
{
	unsigned pos2 = initial_cnf_name.find('.');
	string short_x = initial_cnf_name.substr(0, pos2);
	unsigned pos1 = short_x.find_last_of('_');
	return initial_cnf_name.substr(0, pos1) + ".cnf";
}

double getReducedTimeValueConseqMode( const unsigned reduced_cnf_index, const unsigned solver_index)
{
	double dval = 0;

	for (unsigned i = 0; i < values_vec_vec.size(); i++) {
		string reduced_cnf_name = getReducedCnfName(cnfs_names[i]);
		if (reduced_cnf_name != reduced_cnfs_names[reduced_cnf_index])
			continue;
		if ( values_vec_vec[i][solver_index] >= TRESHOLD_SOLVER_TIME )
			dval += TRESHOLD_SOLVER_TIME;
		else {
			dval += values_vec_vec[i][solver_index];
			break;
		}
	}

	if (ext_solver_name.find("rand") == string::npos)
		dval += CLAUSES_COLLECT_TIME;

	return dval;
}

double getReducedTimeValueParallelMode(const unsigned reduced_cnf_index, const unsigned solver_index)
{
	double dval = TRESHOLD_SOLVER_TIME;

	for (unsigned i = 0; i < values_vec_vec.size(); i++) {
		string reduced_cnf_name = getReducedCnfName(cnfs_names[i]);
		if (reduced_cnf_name != reduced_cnfs_names[reduced_cnf_index])
			continue;
		if (values_vec_vec[i][solver_index] < dval)
			dval = values_vec_vec[i][solver_index];
	}
	
	if (ext_solver_name.find("rand") == string::npos)
		dval += CLAUSES_COLLECT_TIME;
	
	return dval;
}

void writeValuesToCsvFile(string ofile_name, vector<string> local_cnfs_names, vector<vector<double>> local_values_v_v)
{
	string head_str = "Instance";
	for (auto &x : solvers_names)
		head_str += " " + x;
	ofstream ofile(ofile_name);
	ofile << head_str << std::endl;
	for (unsigned i = 0; i < local_values_v_v.size(); i++) {
		ofile << local_cnfs_names[i];
		for (unsigned j = 0; j < local_values_v_v[i].size(); j++)
			ofile << " " << local_values_v_v[i][j];
		ofile << std::endl;
	}
	ofile.close();
}

void writeValuesToSimpleFile(string ofile_name, vector<string> local_cnfs_names, vector<vector<double>> local_values_v_v)
{
	ofstream ofile(ofile_name);
	
	for (unsigned i = 0; i < local_values_v_v.size(); i++) {
		for (unsigned j = 0; j < local_values_v_v[i].size(); j++)
			ofile << solvers_names[j] << " " << local_cnfs_names[i] << " " << local_values_v_v[i][j];
		ofile << endl;
	}

	ofile.close();
}