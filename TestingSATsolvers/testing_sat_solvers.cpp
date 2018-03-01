#ifdef _MPI
#include <mpi.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <stdexcept>

#include "addit_func.h"

using namespace std;

const int SAT = 1;
const int UNSAT = 2;
const int UNKNOWN = 3;
const int WORK_MESSAGE = 1;
const int STOP_MESSAGE = -1;
const int SLEEP_MESSAGE = -2;

using namespace Addit_func;

struct solver_info
{
	string name;
	double avg_time;
	double min_time;
	double max_time;
};

struct mpi_task_solver_cnf
{
	string solver_name;
	string cnf_name;
};

string max_memory_mb_str;
double max_memory_mb;
bool isAlias = false;
string pcs_name = "";

bool conseqProcessing(string solvers_dir, string instances_dir, double maxtime_seconds, string maxtime_seconds_str, const string threads_str);
int solveInstance(string solvers_dir, string instances_dir, string solver_name, string cnf_name, 
	string maxtime_seconds_str, string nof_threads_str);
int solveAliasInstance(string solvers_dir, string instances_dir, string solver_name, string cnf_name,
	string maxtime_seconds_str, string nof_threads_str);
string get_pre_cnf_solver_params_str(string solvers_dir, string solver_name,
	string maxtime_seconds_str, string nof_threads_str);
string get_post_cnf_solver_params_str(string solver_name, string maxtime_seconds_str);
bool controlProcess(int corecount, string solvers_dir, string instances_dir, double maxtime_seconds);
void SendString(string string_to_send, int computing_process);
bool computingProcess(int rank, string solvers_dir, string instances_dir, double maxtime_seconds);
int callMultithreadSolver(int rank, double maxtime_seconds, string solvers_dir, string instances_dir, string solver_name_str, string cnf_name_str);
bool isSkipUnusefulSolver(string solver_name);

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 5;
	argv[1] = "solvers";
	argv[2] = "cnfs";
	argv[3] = "600";
	argv[4] = "4096";
#endif
	
	if ( argc < 3 ) {
		cout << "Usage: prog -solvers_dir [path] -instances_dir [path] -maxseconds [seconds] -maxthreads [threads] -maxmb [mb] -pcs [pcs_name]" << endl;
		return 1;
	}

	string maxtime_seconds_str = "600";
	string max_memory_mb_str = "4096";
	string threads_str = "1";
	string pcs_name = "";
	string solvers_dir = ""; 
	string instances_dir = "";
	for (int i = 1; i < argc; i++) {
		string str = argv[i];
		if (str == "-solvers_dir") {
			if (i < argc - 1)
				solvers_dir = argv[i + 1];
		}
		if (str == "-instances_dir") {
			if (i < argc - 1)
				instances_dir = argv[i + 1];
		}
		if (str == "-maxseconds") {
			if (i < argc - 1)
				maxtime_seconds_str = argv[i + 1];
		}
		if (str == "-maxmb") {
			if (i < argc - 1)
				max_memory_mb_str = argv[i + 1];
		}
		if (str == "-pcs") {
			isAlias = true;
			if (i < argc - 1)
				pcs_name = argv[i + 1];
		}
		if (str == "-maxthreads") {
			if (i < argc - 1)
				threads_str = argv[i + 1];
		}
	}
	
	double maxtime_seconds, max_memory_mb;
	unsigned threads_count = 0;
	istringstream(maxtime_seconds_str) >> maxtime_seconds;
	istringstream(max_memory_mb_str) >> max_memory_mb;
	istringstream(threads_str) >> threads_count;
	cout << "solvers_dir " << solvers_dir << endl;
	cout << "instances_dir " << instances_dir << endl;
	cout << "maxmb " << max_memory_mb << endl;
	cout << "maxseconds " << maxtime_seconds << endl;
	cout << "pcs_name " << pcs_name << endl;
	cout << "threads_count " << threads_count << endl;
	if (pcs_name != "")
		cout << "ALIAS mode" << endl;
	
	string str_to_remove = "./";
	unsigned pos = solvers_dir.find(str_to_remove);
	if (pos != string::npos)
		solvers_dir.erase(pos, str_to_remove.length());
	pos = instances_dir.find(str_to_remove);
	if (pos != string::npos)
		instances_dir.erase(pos, str_to_remove.length());
	
#ifdef _MPI
	cout << "MPI mode " << endl;
	int rank, corecount;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &corecount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (rank == 0)
		controlProcess(corecount, solvers_dir, instances_dir, maxtime_seconds);
	else
		computingProcess(rank, solvers_dir, instances_dir, maxtime_seconds, threads_count);
#else
	cout << "Conseq mode " << endl;
	cout << "solvers_dir " << solvers_dir << endl;
	cout << "instances_dir " << instances_dir << endl;
	cout << "maxtime_seconds " << maxtime_seconds_str << endl;
	conseqProcessing(solvers_dir, instances_dir, maxtime_seconds, maxtime_seconds_str, threads_str);
#endif
	
	return 0;
}

void SendString(string string_to_send, int computing_process)
{
#ifdef _MPI
	int char_arr_len = string_to_send.size();
	char *char_arr;

	//cout << "Sending char_arr_len " << char_arr_len << endl;
	MPI_Send(&char_arr_len, 1, MPI_INT, computing_process, 0, MPI_COMM_WORLD);
	char_arr = new char[char_arr_len + 1];
	char_arr[char_arr_len] = NULL;
	for (unsigned j = 0; j < char_arr_len; j++)
		char_arr[j] = string_to_send[j];
	//cout << "Sending char_arr " << char_arr << endl;
	MPI_Send(char_arr, char_arr_len + 1, MPI_CHAR, computing_process, 0, MPI_COMM_WORLD);
	delete[] char_arr;
#endif
}

bool conseqProcessing(string solvers_dir, string instances_dir, double maxtime_seconds, string maxtime_seconds_str, const string threads_str)
{
	fstream current_out;
	double cur_time, avg_time = 0;
	stringstream sstream;
	string nof_threads_str = "";
	unsigned int nthreads = thread::hardware_concurrency();
	cout << "nthreads " << nthreads << endl;
	sstream << nthreads;
	nof_threads_str = sstream.str();
	sstream.clear(); sstream.str("");
	cout << "nof_threads_str " << nof_threads_str << endl;
	sstream << maxtime_seconds;
	maxtime_seconds_str = sstream.str();
	sstream.clear(); sstream.str("");

	vector<string> solver_files_names = vector<string>();
	vector<string> cnf_files_names = vector<string>();

	if (!Addit_func::getdir(solvers_dir, solver_files_names)) { return false; }
	if (!Addit_func::getdir(instances_dir, cnf_files_names)) { return false; };
	sort(solver_files_names.begin(), solver_files_names.end());
	sort(cnf_files_names.begin(), cnf_files_names.end());

	cout << endl << "solver_files_names :" << endl;
	for (vector<string> ::iterator it = solver_files_names.begin(); it != solver_files_names.end(); it++)
		cout << *it << endl;
	cout << endl << "cnf_files_names :" << endl;
	for (vector<string> ::iterator it = cnf_files_names.begin(); it != cnf_files_names.end(); it++)
		cout << *it << endl;

	vector<solver_info> solver_info_vec;
	solver_info cur_solver_info;
	vector<unsigned> sat_count_vec;
	sat_count_vec.resize(solver_files_names.size());
	for (vector<unsigned> ::iterator it = sat_count_vec.begin(); it != sat_count_vec.end(); it++)
		*it = 0;

	unsigned solved_problems_count;
	vector< vector<string> > solver_cnf_times_str;
	string solver_time_str;
	stringstream convert_sstream;
	solver_cnf_times_str.resize(solver_files_names.size());
	for (unsigned i = 0; i < solver_files_names.size(); i++) {
		double sum_time = 0, min_time = 0, max_time = 0;
		solved_problems_count = 0;
		if (isSkipUnusefulSolver(solver_files_names[i])) {
			cout << "skipping uneseful solver " << solver_files_names[i] << endl;
			continue;
		}
		for (unsigned j = 0; j < cnf_files_names.size(); j++) {
			chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
			int result = solveInstance(solvers_dir, instances_dir, solver_files_names[i], cnf_files_names[j], maxtime_seconds_str, nof_threads_str);
			chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
			chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
			cur_time = time_span.count();
			if (result == SAT)
				sat_count_vec[i]++;
			
			// deal only with finished calculations
			if (cur_time < maxtime_seconds) {
				sum_time += cur_time;
				if (min_time == 0)
					min_time = cur_time;
				else
					min_time = (cur_time < min_time) ? cur_time : min_time;
				if (max_time == 0)
					max_time = cur_time;
				else
					max_time = (cur_time > max_time) ? cur_time : max_time;
				solved_problems_count++;
				cout << "solved_problems_count " << solved_problems_count << endl;
			}

			solver_time_str = cnf_files_names[j];
			convert_sstream << " " << cur_time;
			solver_time_str += convert_sstream.str();
			convert_sstream.str(""); convert_sstream.clear();
			solver_cnf_times_str[i].push_back(solver_time_str);
		}

		if (solved_problems_count)
			avg_time = sum_time / (double)solved_problems_count;

		cout << "cur_avg_time " << avg_time << endl;
		cout << "cur_min_time " << min_time << endl;
		cout << "cur_max_time " << max_time << endl;
		cout << "solved_problems_count " << solved_problems_count << endl;

		string tmp_instances_dir = instances_dir;
		tmp_instances_dir.erase(remove(tmp_instances_dir.begin(), tmp_instances_dir.end(), '.'), tmp_instances_dir.end());
		tmp_instances_dir.erase(remove(tmp_instances_dir.begin(), tmp_instances_dir.end(), '/'), tmp_instances_dir.end());

		string solver_out_file_name = "*out_total_" + solver_files_names[i] + "_" + tmp_instances_dir;
		ofstream solver_out_file(solver_out_file_name.c_str());
		for (unsigned t = 0; t < solver_cnf_times_str[i].size(); t++)
			solver_out_file << solver_cnf_times_str[i][t] << endl;
		solver_out_file.close(); solver_out_file.clear();

		cur_solver_info.name = solver_files_names[i];
		cur_solver_info.avg_time = avg_time;
		cur_solver_info.min_time = min_time;
		cur_solver_info.max_time = max_time;
		solver_info_vec.push_back(cur_solver_info);

		cout << solver_files_names[i] << " : " << sat_count_vec[i] << " sat from " <<
			cnf_files_names.size() << endl;
	}

	cout << "*** Final statistics ***" << endl;
	cout << "Total problems " << cnf_files_names.size() << endl;
	for (vector<solver_info> ::iterator it = solver_info_vec.begin(); it != solver_info_vec.end(); it++) {
		cout << (*it).name << endl;
		cout << "  avg_time " << (*it).avg_time << " s" << endl;
		cout << "  min_time " << (*it).min_time << " s" << endl;
		cout << "  max_time " << (*it).max_time << " s" << endl;
	}

	return true;
}

int solveAliasInstance(string solvers_dir, string instances_dir, string solver_name, string cnf_name,
	string maxtime_seconds_str, string nof_threads_str)
{
	cout << "start solveAliasInstance()" << endl;
	string base_path = Addit_func::exec("echo $PWD");
	base_path.erase(remove(base_path.begin(), base_path.end(), '\r'), base_path.end());
	base_path.erase(remove(base_path.begin(), base_path.end(), '\n'), base_path.end());
	cout << "base path " << base_path << endl;
	
	// launch the script to create a folder for ALIAS and copy files to it
	string system_str = base_path + "/alias_prepare_dir.sh " + 
		 solvers_dir + "/" + solver_name + " " +
		 instances_dir + "/" + cnf_name;
	if (pcs_name != "")
		system_str += " " + pcs_name;
	cout << "alias_prepare_dir.sh command string " << system_str << endl;

	string result_str = Addit_func::exec(system_str);
	cout << "result_str " << result_str << endl;
	
	string alias_launch_path = base_path + "/tmp_" + solver_name + "_" + cnf_name;
	system_str = alias_launch_path + "/alias_ls" +
		" -solver " + alias_launch_path + "/" + solver_name +
		" -cnf " + alias_launch_path + "/" + cnf_name +
		" -script " + alias_launch_path + "/ALIAS.py" +
		" -time " + maxtime_seconds_str;
	if (pcs_name != "")
		system_str += " -pcs " + alias_launch_path + "/" + pcs_name;
	cout << "alias_ls command string " << system_str << endl;
	
	string out_name = base_path + "/out_" + solver_name + "_" + cnf_name;
	fstream out_file;
	out_file.open(out_name, ios_base::out);
	out_file << Addit_func::exec(system_str);
	out_file.close();
	out_file.clear();
	
	return UNKNOWN;
}

int solveInstance(string solvers_dir, string instances_dir, string solver_name, string cnf_name, 
	              string maxtime_seconds_str, string nof_threads_str)
{
	string system_str, current_out_name, str;
	unsigned copy_from, copy_to;
	
#ifdef _MPI
	string cur_path = Addit_func::exec("echo $PWD");
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	current_out_name = cur_path + "/out_" + solver_name + "_" + cnf_name;
	system_str = get_pre_cnf_solver_params_str(solvers_dir, solver_name, maxtime_seconds_str, nof_threads_str) +
		" " + cur_path + "/" + instances_dir + "/" + cnf_name + get_post_cnf_solver_params_str(solver_name, maxtime_seconds_str);
#else
	current_out_name = "out_" + solver_name + "_" + cnf_name;
	system_str = get_pre_cnf_solver_params_str(solvers_dir, solver_name, maxtime_seconds_str, nof_threads_str) +
		" ./" + instances_dir + "/" + cnf_name + get_post_cnf_solver_params_str(solver_name, maxtime_seconds_str);
#endif
	
	cout << system_str << endl;
	// + " &> ./" + current_out_name;
	//cout << system_str << endl;
	//cout << "system_result_stream" << endl;
	//cout << system_result_stream.str() << endl;
	//system( system_str.c_str( ) );
	fstream current_out;
	current_out.open(current_out_name, ios_base::out);
	if (!current_out.is_open()) {
		cout << current_out_name << " wasn't opened" << endl;
		cout << "try to open it one more time" << endl;
		current_out.open(current_out_name, ios_base::out);
	}
	if (!current_out.is_open()) {
		cerr << "couldn't open file " << current_out_name << endl;
		exit(1);
	}
	cout << "created out file " << current_out_name << endl;
	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
	current_out << Addit_func::exec(system_str);
	current_out.clear();
	current_out.close();
	
	current_out.open(current_out_name, ios_base::in);
	if (!current_out.is_open()) {
		cerr << "couldn't open file " << current_out_name << endl;
		exit(1);
	}
	
	stringstream sstream;
	int result = UNKNOWN;
	double cur_time = 0.0;
	while (getline(current_out, str)) {
		if (str.find("c SATISFIABLE") != string::npos) {
			result = SAT;
			//sat_count_vec[i]++;
			//cout << "SAT found" << endl;
			//cout << "current_out_name " << current_out_name << endl;
		}
		else if (str.find("UNSAT") != string::npos)
			result = UNSAT;
		bool isTimeStr = true;
		if (str.find("CPU time") != string::npos) {
			copy_from = str.find(":") + 2;
			copy_to = str.find(" s") - 1;
		}
		else if ((str.find("wall clock time, ") != string::npos) && // treengeling format
			(str.find("process time") != string::npos))
		{
			copy_from = str.find("wall clock time, ") + 17;
			copy_to = str.find(" process time") - 1;
		}
		else if (str.find("process time") != string::npos) { // plingeling format
			copy_from = str.find("c ") + 2;
			copy_to = str.find(" process time") - 1;
		}
		else if ((str.find("seconds") != string::npos) && // lingeling format
			(str.find("MB") != string::npos) &&
			(str.size() < 30)) {
			copy_from = str.find("c ") + 2;
			copy_to = str.find(" seconds") - 1;
		}
		else if (str.find("c Running time=") != string::npos) { // glueSplit_clasp format
			copy_from = str.find("c Running time=") + 15;
			copy_to = str.size() - 1;
		}
		else if (str.find(" fault") != string::npos) {
			cerr << "fault detected" << endl;
			exit(-1);
		}
		else
			isTimeStr = false;
		if (isTimeStr) {
			cout << "time str " << str << endl;
			str = str.substr(copy_from, (copy_to - copy_from + 1));
			sstream << str;
			sstream >> cur_time;
			sstream.str(""); sstream.clear();
			break;
		}
	}
	current_out.close();
	return result;
}

bool controlProcess(int corecount, string solvers_dir, string instances_dir, double maxtime_seconds)
{
#ifdef _MPI
	cout << "control MPI process" << endl;
	cout << "solvers_dir " << solvers_dir << endl;
	cout << "instances_dir " << instances_dir << endl;
	cout << "maxtime_seconds " << maxtime_seconds << endl;

	cout << "*** Stage 1. Collect hosts names." << endl;

	int buffer_size = 0;
	char *buffer;
	MPI_Status status;
	vector<string> hosts_names_vec;
	hosts_names_vec.resize(corecount-1);
	for (int i = 1; i < corecount; i++) {
		MPI_Probe(i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_CHAR, &buffer_size);
		buffer = new char[buffer_size];
		MPI_Recv(buffer, buffer_size, MPI_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		hosts_names_vec[i-1] = buffer;
		delete[] buffer;
	}
	
	cout << "hosts_names : " << endl; 
	for (auto &x : hosts_names_vec)
		cout << x << endl;

	cout << "*** Stage 2. Solve all instances." << endl;
	
	vector<string> checked_hosts_names;
	vector<int> computing_process_vec; // the vector of processes which have to launch multithread solvers
	for (unsigned i = 0; i < hosts_names_vec.size(); i++)
		if (find(checked_hosts_names.begin(), checked_hosts_names.end(), hosts_names_vec[i]) == checked_hosts_names.end()) {
			checked_hosts_names.push_back(hosts_names_vec[i]);
			computing_process_vec.push_back(i+1);
		}
	
	cout << "computing_process_vec " << endl;
	for (auto &x : computing_process_vec)
		cout << x << " ";
	cout << endl;
	unsigned computing_processes = computing_process_vec.size();
	
	string system_str, cur_process_dir_name;

	double control_process_solving_time = MPI_Wtime();
	vector<string> solver_files_names = vector<string>();
	vector<string> cnf_files_names = vector<string>();
	
	vector<string> solved_instances;
	string cur_path = Addit_func::exec("echo $PWD");
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	solvers_dir = cur_path + "/" + solvers_dir;
	cout << "full solvers_dir " << solvers_dir << endl;
	instances_dir = cur_path + "/" + instances_dir;
	cout << "full instances_dir " << instances_dir << endl;
	if (!Addit_func::getdir(instances_dir, cnf_files_names)) { return false; };
	if (!Addit_func::getdir(solvers_dir, solver_files_names)) { return false; }
	sort(solver_files_names.begin(), solver_files_names.end());
	sort(cnf_files_names.begin(), cnf_files_names.end());

	cout << endl << "solver_files_names :" << endl;
	for (vector<string> ::iterator it = solver_files_names.begin(); it != solver_files_names.end(); it++)
		cout << *it << endl;
	cout << endl << "cnf_files_names :" << endl;
	for (vector<string> ::iterator it = cnf_files_names.begin(); it != cnf_files_names.end(); it++)
		cout << *it << endl;

	vector<mpi_task_solver_cnf> tasks_vec;
	mpi_task_solver_cnf cur_task;
	for (auto &x : solver_files_names)
		for (auto &y : cnf_files_names) {
			cur_task.solver_name = x;
			cur_task.cnf_name = y;
			tasks_vec.push_back(cur_task);
		}
	cout << "tasks_vec.size() " << tasks_vec.size() << endl;
	
	int sent_tasks = 0, solved_tasks = 0;
	int first_send_tasks = (computing_processes < tasks_vec.size()) ? computing_processes : tasks_vec.size();
	cout << "first_send_tasks " << first_send_tasks << endl;

	// send maxtime_seconds data once for 1 process per node
	// for the others processes from a node send the sleep message
	unsigned interrupted = 0;
	for (int i = 1; i < corecount; i++) {
		int message = 0;
		if (find(computing_process_vec.begin(), computing_process_vec.end(), i) != computing_process_vec.end())
			message = WORK_MESSAGE;
		else
			message = SLEEP_MESSAGE;
		MPI_Send(&message, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	}
	
	// send first part of tasks
	for (int i = 0; i < first_send_tasks; i++) {
		cout << "Sending task " << i << endl;
		MPI_Send(&i, 1, MPI_INT, computing_process_vec[i], 0, MPI_COMM_WORLD);
		cout << "Sending solver_name " << tasks_vec[i].solver_name << endl;
		SendString(tasks_vec[i].solver_name, computing_process_vec[i]);
		cout << "Sending cnf_name " << tasks_vec[i].cnf_name << endl;
		SendString(tasks_vec[i].cnf_name, computing_process_vec[i]);
	}
	sent_tasks = first_send_tasks;
	
	double process_solving_time;
	int process_task_index;
	vector<double> process_solving_time_vec;
	vector<int> result_vec;
	process_solving_time_vec.resize(tasks_vec.size());
	result_vec.resize(tasks_vec.size());
	for (auto &x : process_solving_time_vec)
		x = -1;
	for (auto &x : result_vec)
		x = UNKNOWN;
	ofstream ofile;
	int result = 0;
	int iprobe_message = 0;

	while (solved_tasks < tasks_vec.size()) {
		/*iprobe_message = 0;
		for (;;) {
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &iprobe_message, &status);
			if (iprobe_message)
				break; // if any message from computing processes, catch it
			sleep(10);
		}*/ // don't sleep, just use 1 core of the first node (this node will not be used for solvers launching)
		
		MPI_Recv(&process_task_index, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		cout << "Recieved process_task_index " << process_task_index << endl;
		MPI_Recv(&process_solving_time, 1, MPI_DOUBLE, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		cout << "Recieved process_solving_time " << process_solving_time << endl;
		MPI_Recv(&result, 1, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		cout << "Recieved result " << result << endl;
		process_solving_time_vec[process_task_index] = process_solving_time;
		result_vec[process_task_index] = result;
		if (process_solving_time >= maxtime_seconds)
			interrupted++;
		solved_tasks++;
		cout << "solved_tasks " << solved_tasks << endl;
		if (interrupted)
			cout << "interrupted " << solved_tasks << endl;
		
		if (sent_tasks < tasks_vec.size()) {
			// send a new task if one exists
			cout << "Sending task " << sent_tasks << endl;
			MPI_Send(&sent_tasks, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			cout << "Sending solver_name " << tasks_vec[sent_tasks].solver_name << endl;
			SendString(tasks_vec[sent_tasks].solver_name, status.MPI_SOURCE);
			cout << "Sending cnf_name " << tasks_vec[sent_tasks].cnf_name << endl;
			SendString(tasks_vec[sent_tasks].cnf_name, status.MPI_SOURCE);
			sent_tasks++;
			cout << "sent_tasks " << sent_tasks << endl;
		}
		//else {
			// no new tasks, tell computing process to free its resources
		//	MPI_Send(&STOP_MESSAGE, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		//	cout << "stop-message has been sent to computing process " << status.MPI_SOURCE << endl;
		//}  send stop-message on all processes including sleeping ones
		
		ofile.open("testing_sat_solvers_out", ios_base::out);
		ofile << "instance result time" << endl;
		for (int i = 0; i < tasks_vec.size(); i++)
			ofile << tasks_vec[i].solver_name << " " 
			      << tasks_vec[i].cnf_name << " " 
			      << result_vec[i] << " " 
			      << process_solving_time_vec[i] << endl;
		ofile.close(); ofile.clear();
	}
	
	cout << "End of the control process" << endl;
	control_process_solving_time = MPI_Wtime() - control_process_solving_time;
	cout << "control_process_solving_time " << control_process_solving_time << endl;

	MPI_Request mpi_request;
	cout << "sending stop-messages on all processes " << endl;
	for (int i = 1; i < corecount; i++)
		MPI_Isend(&STOP_MESSAGE, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &mpi_request);
	
	double min = 1e50, max = 0, med = -1, sum = 0;
	for (auto &x : process_solving_time_vec) {
		sum += x;
		min = x < min ? x : min;
		max = x > max ? x : max;
	}
	med = sum / process_solving_time_vec.size();
	cout << "min time " << min << endl;
	cout << "max time " << max << endl;
	cout << "med time " << med << endl;
	unsigned sat_count = 0, unsat_count = 0, unknown_count = 0;
	for (auto &x : result_vec) {
		if (x == SAT)
			sat_count++;
		else if (x == UNSAT)
			unsat_count++;
		else if (x == UNKNOWN)
			unknown_count++;
	}
	cout << "sat_count " << sat_count << endl;
	cout << "unsat_count " << unsat_count << endl;
	cout << "unknown_count " << unknown_count << endl;

	MPI_Finalize();
#endif
	return true;
}

bool computingProcess(int rank, string solvers_dir, string instances_dir)
{
#ifdef _MPI
	int hostNameLength = 0;
	char *hostName = new char[MPI_MAX_PROCESSOR_NAME];
	MPI_Get_processor_name(hostName, &hostNameLength);
	//cout << "sending hostName " << hostName << " with hostNameLength " << hostNameLength << endl;
	MPI_Send(hostName, hostNameLength, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	delete[] hostName;

	MPI_Status status;
	int process_task_index;
	double process_solving_time = 0;
	string cnf_instance_name;
	int cnf_name_char_arr_len = 0;
	char *cnf_name_char_arr;
	int solver_name_char_arr_len = 0;
	char *solver_name_char_arr;
	int first_message;

	// get maxtime_seconds once
	MPI_Recv(&first_message, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	if (rank == 1)
		cout << "Recevied maxtime_seconds " << maxtime_seconds << endl;
	
	bool isFinalize = false;
	// sleep and let other process from the node to launch multithread solver
	if (first_message == SLEEP_MESSAGE) {
		cout << "computing process " << rank << " starts to sleep" << endl;
		for (;;) {
			sleep(60);
			int iprobe_message = 0;
			MPI_Iprobe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &iprobe_message, &status);
			if (iprobe_message) {
				isFinalize = true;
				cout << "Received stop message on computing process " << rank << endl;
				break; // if any message from computing processes, catch it
			}
		}
	}
	
	int result;
	
	for (;;) {
		if (isFinalize)
			break;
		MPI_Recv(&process_task_index, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (rank == 1)
			cout << "Received process_task_index " << process_task_index << endl;
		if (process_task_index == STOP_MESSAGE) {
			cout << "Received stop message on computing process " << rank << endl;
			break;
		}
		MPI_Recv(&solver_name_char_arr_len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (rank == 1)
			cout << "solver_name_char_arr_len " << solver_name_char_arr_len << endl;
		solver_name_char_arr = new char[solver_name_char_arr_len + 1];
		MPI_Recv(solver_name_char_arr, solver_name_char_arr_len + 1,
			MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		string solver_name_str = solver_name_char_arr;
		delete[] solver_name_char_arr;
		MPI_Recv(&cnf_name_char_arr_len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (rank == 1)
			cout << "cnf_name_char_arr_len " << cnf_name_char_arr_len << endl;
		cnf_name_char_arr = new char[cnf_name_char_arr_len + 1];
		MPI_Recv(cnf_name_char_arr, cnf_name_char_arr_len + 1,
			MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		string cnf_name_str = cnf_name_char_arr;
		delete[] cnf_name_char_arr;
		if (rank == 1) {
			cout << "Received cnf_name_str " << cnf_name_str << endl;
			cout << "Received solver_name_str " << solver_name_str << endl;
		}
		
		process_solving_time = MPI_Wtime();
		// solving with received Tfact
		result = callMultithreadSolver(rank, maxtime_seconds, solvers_dir, instances_dir, solver_name_str, cnf_name_str);
		process_solving_time = MPI_Wtime() - process_solving_time;

		MPI_Send(&process_task_index,   1, MPI_INT,    0, 0, MPI_COMM_WORLD);
		MPI_Send(&process_solving_time, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&result,               1, MPI_INT,    0, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
#endif
	return true;
}

int callMultithreadSolver( int rank, 
	                       double maxtime_seconds, 
	                       string solvers_dir, 
	                       string instances_dir, 
	                       string solver_name_str, 
	                       string cnf_name_str)
{
	string maxtime_seconds_str;
	stringstream sstream;
	sstream << maxtime_seconds;
	maxtime_seconds_str = sstream.str();
	sstream.str(""); sstream.clear();
	int cores_per_node = std::thread::hardware_concurrency();
	//cout << "cores_per_node " << cores_per_node << endl;
	sstream << cores_per_node;
	string nof_threads_str = sstream.str();
	sstream.str(""); sstream.clear();
	int result;

	if (isAlias)
		result = solveAliasInstance(solvers_dir, instances_dir, solver_name_str, cnf_name_str, maxtime_seconds_str, nof_threads_str);
	else
		result = solveInstance(solvers_dir, instances_dir, solver_name_str, cnf_name_str, maxtime_seconds_str, nof_threads_str);

	return result;
}

string get_pre_cnf_solver_params_str(string solvers_dir, string solver_name,
	string maxtime_seconds_str, string nof_threads_str)
{
	string solver_params_str;
	string result_str;
	bool isTimeLimit = false;

	if ( ( (solver_name.find("minisat") != string::npos) ||
		   (solver_name.find("rokk") != string::npos) ) &&
		   (solver_name.find("cryptominisat") == string::npos) )
	{
		solver_params_str = "-cpu-lim=" + maxtime_seconds_str;
		isTimeLimit = true;
	}
	cout << "isTimeLimit " << isTimeLimit << endl;
	
	if ((solver_name.find("plingeling") != string::npos) ||
		(solver_name.find("treengeling") != string::npos))
		solver_params_str += " -t " + nof_threads_str;
	else if (solver_name.find("cryptominisat") != string::npos)
		solver_params_str += "--threads=" + nof_threads_str;
	else if (solver_name.find("syrup") != string::npos)
		solver_params_str += "-nthreads=" + nof_threads_str;
	else if ((solver_name.find("hordesat") != string::npos) ||
		     (solver_name.find("painless") != string::npos))
		solver_params_str += "-c=" + nof_threads_str;
	else if (solver_name.find("cvc4") != string::npos)
		solver_params_str += "--lang smtlib2";
	else if (solver_name.find("z3") != string::npos)
		solver_params_str += "-smt2";
	
	double max_memory_gb = max_memory_mb / 1024;
	stringstream sstream;
	sstream << max_memory_gb;
	string max_memory_gb_str = sstream.str();
	if (solver_name.find("painless") != string::npos)
		solver_params_str += " -max-memory=" + max_memory_gb_str;
	else if (solver_name.find("syrup") != string::npos)
		solver_params_str += " -maxmemory=" + max_memory_mb_str;
	
#ifdef _MPI
	string cur_path = Addit_func::exec("echo $PWD");
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	result_str = cur_path + "/timelimit -t " + maxtime_seconds_str + " -T 1 " + cur_path + "/" +
		solvers_dir + "/" + solver_name + " " + solver_params_str;
#else
	if (!isTimeLimit) {
		result_str = "./timelimit -t " + maxtime_seconds_str + " -T 1 " + "./" +
			solvers_dir + "/" + solver_name;
	}
	else {
		result_str = "./" + solvers_dir + "/" + solver_name + " " + solver_params_str;
	}
#endif

	return result_str;
}

string get_post_cnf_solver_params_str(string solver_name, string maxtime_seconds_str )
{
	string result_str;
	if ( (solver_name.find("CSCC") != string::npos)   || 
	     (solver_name.find("DCCAlm") != string::npos) ||
		 (solver_name.find("DCCASat") != string::npos)
		)
	{
		result_str = " 1";
	}
	else if (solver_name.find("dimetheus") != string::npos) {
		result_str = " -guide 6";
	}
	else if (solver_name.find(".sh") != string::npos) {
		result_str = " " + maxtime_seconds_str;
	}
	return result_str;
}

bool isSkipUnusefulSolver(string solver_name)
{
	if (solver_name.find("WalkSATlm2013") != string::npos)
		return true;

	return false;
}