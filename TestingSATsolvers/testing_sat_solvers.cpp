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

const unsigned CORES_PER_NODE = 36;
const int SAT = 1;
const int UNSAT = 2;
const int UNKNOWN = 3;
const int STOP_MESSAGE = -1;
const double SLEEP_MESSAGE = -2;

using namespace Addit_func;

struct solver_info
{
	std::string name;
	double avg_time;
	double min_time;
	double max_time;
};

struct mpi_task_solver_cnf
{
	string solver_name;
	string cnf_name;
};

bool conseqProcessing(std::string solvers_dir, std::string cnfs_dir, double maxtime_seconds, 
	                  std::string maxtime_seconds_str);
int solveInstance(string solvers_dir, string cnfs_dir, string solver_name, string cnf_name, string maxtime_seconds_str, string nof_threads_str);
std::string get_pre_cnf_solver_params_str(std::string solvers_dir, std::string solver_name,
	std::string maxtime_seconds_str, std::string nof_threads_str);
std::string get_post_cnf_solver_params_str(std::string solver_name);
bool controlProcess(int corecount, std::string solvers_dir, std::string cnfs_dir, double maxtime_seconds);
void SendString(string string_to_send, int computing_process);
bool computingProcess(int rank, string solvers_dir, string cnfs_dir, double maxtime_seconds);
int callMultithreadSolver(int rank, double maxtime_seconds, string solvers_dir, string cnfs_dir, string solver_name_str, string cnf_name_str);
bool isSkipUnusefulSolver(std::string solver_name);

int main( int argc, char **argv )
{
#ifdef _DEBUG
	argc = 3;
	argv[1] = "solvers";
	argv[2] = "cnfs";
	argv[3] = "-mpi";
#endif
	
	if ( argc < 3 ) {
		std::cout << "Usage: [solvers_path] [cnfs_path] [maxtime_seconds_one_problem]" << std::endl;
		return 1;
	}

	std::string maxtime_seconds_str;
	if ( argc == 3 ) {
		maxtime_seconds_str = "600"; 
		std::cout << "maxtime_seconds was set to default value of 600 seconds" << std::endl;
	} else
		maxtime_seconds_str = argv[3];

	double maxtime_seconds; 
	std::istringstream(maxtime_seconds_str) >> maxtime_seconds;

	std::string solvers_dir, cnfs_dir;
	solvers_dir = argv[1];
	cnfs_dir = argv[2];
	std::string str_to_remove = "./";
	unsigned pos = solvers_dir.find(str_to_remove);
	if (pos != std::string::npos)
		solvers_dir.erase(pos, str_to_remove.length());
	pos = cnfs_dir.find(str_to_remove);
	if (pos != std::string::npos)
		cnfs_dir.erase(pos, str_to_remove.length());
		
#ifdef _MPI
	std::cout << "MPI mode " << std::endl;
	int rank, corecount;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &corecount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
		controlProcess(corecount, solvers_dir, cnfs_dir, maxtime_seconds);
	else
		computingProcess(rank, solvers_dir, cnfs_dir, maxtime_seconds);
#else
	std::cout << "Conseq mode " << std::endl;
	std::cout << "solvers_dir " << solvers_dir << std::endl;
	std::cout << "cnfs_dir " << cnfs_dir << std::endl;
	std::cout << "maxtime_seconds " << maxtime_seconds_str << std::endl;
	conseqProcessing(solvers_dir, cnfs_dir, maxtime_seconds, maxtime_seconds_str);
#endif
	
	return 0;
}

bool conseqProcessing(std::string solvers_dir, std::string cnfs_dir, double maxtime_seconds, std::string maxtime_seconds_str)
{
	std::fstream current_out;
	double cur_time, avg_time = 0;
	std::stringstream sstream;
	std::string nof_threads_str = "";
	unsigned int nthreads = std::thread::hardware_concurrency();
	std::cout << "nthreads " << nthreads << std::endl;
	sstream << nthreads;
	nof_threads_str = sstream.str();
	sstream.clear(); sstream.str("");
	std::cout << "nof_threads_str " << nof_threads_str << std::endl;
	sstream << maxtime_seconds;
	maxtime_seconds_str = sstream.str();
	sstream.clear(); sstream.str("");

	std::vector<std::string> solver_files_names = std::vector<std::string>();
	std::vector<std::string> cnf_files_names = std::vector<std::string>();

	if (!Addit_func::getdir(solvers_dir, solver_files_names)) { return false; }
	if (!Addit_func::getdir(cnfs_dir, cnf_files_names)) { return false; };
	std::sort(solver_files_names.begin(), solver_files_names.end());
	std::sort(cnf_files_names.begin(), cnf_files_names.end());

	std::cout << std::endl << "solver_files_names :" << std::endl;
	for (std::vector<std::string> ::iterator it = solver_files_names.begin(); it != solver_files_names.end(); it++)
		std::cout << *it << std::endl;
	std::cout << std::endl << "cnf_files_names :" << std::endl;
	for (std::vector<std::string> ::iterator it = cnf_files_names.begin(); it != cnf_files_names.end(); it++)
		std::cout << *it << std::endl;

	std::vector<solver_info> solver_info_vec;
	solver_info cur_solver_info;
	std::vector<unsigned> sat_count_vec;
	sat_count_vec.resize(solver_files_names.size());
	for (std::vector<unsigned> ::iterator it = sat_count_vec.begin(); it != sat_count_vec.end(); it++)
		*it = 0;

	unsigned solved_problems_count;
	std::vector< std::vector<std::string> > solver_cnf_times_str;
	std::string solver_time_str;
	std::stringstream convert_sstream;
	solver_cnf_times_str.resize(solver_files_names.size());
	for (unsigned i = 0; i < solver_files_names.size(); i++) {
		double sum_time = 0, min_time = 0, max_time = 0;
		solved_problems_count = 0;
		if (isSkipUnusefulSolver(solver_files_names[i])) {
			std::cout << "skipping uneseful solver " << solver_files_names[i] << std::endl;
			continue;
		}
		for (unsigned j = 0; j < cnf_files_names.size(); j++) {
			std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
			int result = solveInstance(solvers_dir, cnfs_dir, solver_files_names[i], cnf_files_names[j], maxtime_seconds_str, nof_threads_str);
			std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
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
				std::cout << "solved_problems_count " << solved_problems_count << std::endl;
			}

			solver_time_str = cnf_files_names[j];
			convert_sstream << " " << cur_time;
			solver_time_str += convert_sstream.str();
			convert_sstream.str(""); convert_sstream.clear();
			solver_cnf_times_str[i].push_back(solver_time_str);
		}

		if (solved_problems_count)
			avg_time = sum_time / (double)solved_problems_count;

		std::cout << "cur_avg_time " << avg_time << std::endl;
		std::cout << "cur_min_time " << min_time << std::endl;
		std::cout << "cur_max_time " << max_time << std::endl;
		std::cout << "solved_problems_count " << solved_problems_count << std::endl;

		std::string tmp_cnfs_dir = cnfs_dir;
		tmp_cnfs_dir.erase(std::remove(tmp_cnfs_dir.begin(), tmp_cnfs_dir.end(), '.'), tmp_cnfs_dir.end());
		tmp_cnfs_dir.erase(std::remove(tmp_cnfs_dir.begin(), tmp_cnfs_dir.end(), '/'), tmp_cnfs_dir.end());

		std::string solver_out_file_name = "*out_total_" + solver_files_names[i] + "_" + tmp_cnfs_dir;
		std::ofstream solver_out_file(solver_out_file_name.c_str());
		for (unsigned t = 0; t < solver_cnf_times_str[i].size(); t++)
			solver_out_file << solver_cnf_times_str[i][t] << std::endl;
		solver_out_file.close(); solver_out_file.clear();

		cur_solver_info.name = solver_files_names[i];
		cur_solver_info.avg_time = avg_time;
		cur_solver_info.min_time = min_time;
		cur_solver_info.max_time = max_time;
		solver_info_vec.push_back(cur_solver_info);

		std::cout << solver_files_names[i] << " : " << sat_count_vec[i] << " sat from " <<
			cnf_files_names.size() << std::endl;
	}

	std::cout << "*** Final statistics ***" << std::endl;
	std::cout << "Total problems " << cnf_files_names.size() << std::endl;
	for (std::vector<solver_info> ::iterator it = solver_info_vec.begin(); it != solver_info_vec.end(); it++) {
		std::cout << (*it).name << std::endl;
		std::cout << "  avg_time " << (*it).avg_time << " s" << std::endl;
		std::cout << "  min_time " << (*it).min_time << " s" << std::endl;
		std::cout << "  max_time " << (*it).max_time << " s" << std::endl;
	}

	return true;
}

int solveInstance(string solvers_dir, string cnfs_dir, string solver_name, string cnf_name, string maxtime_seconds_str, string nof_threads_str)
{
	std::string system_str, current_out_name, str;
	unsigned copy_from, copy_to;
	
#ifdef _MPI
	string cur_path = Addit_func::exec("echo $PWD");
	cur_path.erase(std::remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(std::remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	current_out_name = cur_path + "/out_" + solver_name + "_" + cnf_name;
	system_str = get_pre_cnf_solver_params_str(solvers_dir, solver_name, maxtime_seconds_str, nof_threads_str) +
		" " + cur_path + "/" + cnfs_dir + "/" + cnf_name + get_post_cnf_solver_params_str(solver_name);
#else
	current_out_name = "out_" + solver_name + "_" + cnf_name;
	system_str = get_pre_cnf_solver_params_str(solvers_dir, solver_name, maxtime_seconds_str, nof_threads_str) +
		" ./" + cnfs_dir + "/" + cnf_name + get_post_cnf_solver_params_str(solver_name);
#endif
	
	std::cout << system_str << std::endl;
	// + " &> ./" + current_out_name;
	//std::cout << system_str << std::endl;
	//cout << "system_result_stream" << endl;
	//cout << system_result_stream.str() << endl;
	//system( system_str.c_str( ) );
	fstream current_out; 
	current_out.open(current_out_name.c_str(), std::ios_base::out);
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	current_out << Addit_func::exec(system_str);
	current_out.close();

	current_out.open(current_out_name.c_str(), std::ios_base::in);
	
	stringstream sstream;
	int result = UNKNOWN;
	double cur_time = 0.0;
	while (getline(current_out, str)) {
		if (str.find("SATISFIABLE") != std::string::npos) {
			result = SAT;
			//sat_count_vec[i]++;
			//std::cout << "SAT found" << std::endl;
			//std::cout << "current_out_name " << current_out_name << std::endl;
		}
		else if (str.find("UNSAT") != std::string::npos)
			result = UNSAT;
		bool isTimeStr = true;
		if (str.find("CPU time") != std::string::npos) {
			copy_from = str.find(":") + 2;
			copy_to = str.find(" s") - 1;
		}
		else if ((str.find("wall clock time, ") != std::string::npos) && // treengeling format
			(str.find("process time") != std::string::npos))
		{
			copy_from = str.find("wall clock time, ") + 17;
			copy_to = str.find(" process time") - 1;
		}
		else if (str.find("process time") != std::string::npos) { // plingeling format
			copy_from = str.find("c ") + 2;
			copy_to = str.find(" process time") - 1;
		}
		else if ((str.find("seconds") != std::string::npos) && // lingeling format
			(str.find("MB") != std::string::npos) &&
			(str.size() < 30)) {
			copy_from = str.find("c ") + 2;
			copy_to = str.find(" seconds") - 1;
		}
		else if (str.find("c Running time=") != std::string::npos) { // glueSplit_clasp format
			copy_from = str.find("c Running time=") + 15;
			copy_to = str.size() - 1;
		}
		else
			isTimeStr = false;
		if (isTimeStr) {
			std::cout << "time str " << str << std::endl;
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

bool controlProcess(int corecount, std::string solvers_dir, std::string cnfs_dir, double maxtime_seconds)
{
#ifdef _MPI
	cout << "control MPI process" << endl;
	std::cout << "solvers_dir " << solvers_dir << std::endl;
	std::cout << "cnfs_dir " << cnfs_dir << std::endl;
	std::cout << "maxtime_seconds " << maxtime_seconds << std::endl;

	std::string system_str, cur_process_dir_name;

	double control_process_solving_time = MPI_Wtime();
	std::vector<std::string> solver_files_names = std::vector<std::string>();
	std::vector<std::string> cnf_files_names = std::vector<std::string>();
	
	std::vector<std::string> solved_instances;
	string cur_path = Addit_func::exec("echo $PWD");
	cur_path.erase(std::remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(std::remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	solvers_dir = cur_path + "/" + solvers_dir;
	cout << "full solvers_dir " << solvers_dir << endl;
	cnfs_dir = cur_path + "/" + cnfs_dir;
	cout << "full cnfs_dir " << cnfs_dir << endl;
	if (!Addit_func::getdir(cnfs_dir, cnf_files_names)) { return false; };
	if (!Addit_func::getdir(solvers_dir, solver_files_names)) { return false; }
	sort(solver_files_names.begin(), solver_files_names.end());
	sort(cnf_files_names.begin(), cnf_files_names.end());

	std::cout << std::endl << "solver_files_names :" << std::endl;
	for (std::vector<std::string> ::iterator it = solver_files_names.begin(); it != solver_files_names.end(); it++)
		std::cout << *it << std::endl;
	std::cout << std::endl << "cnf_files_names :" << std::endl;
	for (std::vector<std::string> ::iterator it = cnf_files_names.begin(); it != cnf_files_names.end(); it++)
		std::cout << *it << std::endl;

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

	std::vector<int> computing_process_vec; // vector of processes which must launch a multithread solver
	if (CORES_PER_NODE > 1) {
		for (unsigned i = 0; i < (corecount / CORES_PER_NODE); i++)
			computing_process_vec.push_back(i*CORES_PER_NODE + 1);
	}
	else {
		for (unsigned i = 1; i < corecount; i++)
			computing_process_vec.push_back(i);
	}
	std::cout << "computing_process_vec " << std::endl;
	for (auto &x : computing_process_vec)
		std::cout << x << " ";
	std::cout << std::endl;
	unsigned computing_processes = computing_process_vec.size();

	int first_send_tasks = (computing_processes < tasks_vec.size()) ? computing_processes : tasks_vec.size();
	std::cout << "first_send_tasks " << first_send_tasks << std::endl;
	
	// send maxtime_seconds data once for 1 process per node
	// for the others processes from a node send the sleep message
	unsigned interrupted = 0;
	double one_d = 1;
	for (int i = 1; i < corecount; i++) {
		if (std::find(computing_process_vec.begin(), computing_process_vec.end(), i) != computing_process_vec.end())
			MPI_Send(&one_d, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
		else {
			MPI_Send(&SLEEP_MESSAGE, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD); // process for sleeping
			cout << "sleep-message was sent to the computing process " << i << endl;
		}
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
	
	MPI_Status status;
	double process_solving_time;
	int process_task_index;
	std::vector<double> process_solving_time_vec;
	std::vector<int> result_vec;
	process_solving_time_vec.resize(tasks_vec.size());
	result_vec.resize(tasks_vec.size());
	for (auto &x : process_solving_time_vec)
		x = -1;
	for (auto &x : result_vec)
		x = UNKNOWN;
	std::ofstream ofile;
	int result;

	while (solved_tasks < tasks_vec.size()) {
		MPI_Recv(&process_task_index, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		std::cout << "Recieved process_task_index " << process_task_index << std::endl;
		MPI_Recv(&process_solving_time, 1, MPI_DOUBLE, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		std::cout << "Recieved process_solving_time " << process_solving_time << std::endl;
		MPI_Recv(&result, 1, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		std::cout << "Recieved result " << result << std::endl;
		process_solving_time_vec[process_task_index] = process_solving_time;
		result_vec[process_task_index] = result;
		if (process_solving_time >= maxtime_seconds)
			interrupted++;
		solved_tasks++;
		std::cout << "solved_tasks " << solved_tasks << std::endl;
		if (interrupted)
			std::cout << "interrupted " << solved_tasks << std::endl;
		
		if (sent_tasks < tasks_vec.size()) {
			// send a new task if one exists
			cout << "Sending task " << sent_tasks << endl;
			MPI_Send(&sent_tasks, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			cout << "Sending solver_name " << tasks_vec[sent_tasks].solver_name << endl;
			SendString(tasks_vec[sent_tasks].solver_name, status.MPI_SOURCE);
			cout << "Sending cnf_name " << tasks_vec[sent_tasks].cnf_name << endl;
			SendString(tasks_vec[sent_tasks].cnf_name, status.MPI_SOURCE);
			sent_tasks++;
			std::cout << "sent_tasks " << sent_tasks << std::endl;
		}
		else {
			// no new tasks, tell computing process to free its resources
			MPI_Send(&STOP_MESSAGE, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			std::cout << "stop-message has been sent to computing process " << status.MPI_SOURCE << std::endl;
		}
		
		ofile.open("testing_sat_solvers_out", std::ios_base::out);
		ofile << "instance result time" << std::endl;
		for (int i = 0; i < tasks_vec.size(); i++)
			ofile << tasks_vec[i].solver_name << " " 
			      << tasks_vec[i].cnf_name << " " 
			      << result_vec[i] << " " 
			      << process_solving_time_vec[i] << std::endl;
		ofile.close(); ofile.clear();
	}
	
	std::cout << "End of the control process" << std::endl;
	control_process_solving_time = MPI_Wtime() - control_process_solving_time;
	std::cout << "control_process_solving_time " << control_process_solving_time << std::endl;
	
	double min = 1e50, max = 0, med = -1, sum = 0;
	for (auto &x : process_solving_time_vec) {
		sum += x;
		min = x < min ? x : min;
		max = x > max ? x : max;
	}
	med = sum / process_solving_time_vec.size();
	std::cout << "min time " << min << std::endl;
	std::cout << "max time " << max << std::endl;
	std::cout << "med time " << med << std::endl;
	unsigned sat_count = 0, unsat_count = 0, unknown_count = 0;
	for (auto &x : result_vec) {
		if (x == SAT)
			sat_count++;
		else if (x == UNSAT)
			unsat_count++;
		else if (x == UNKNOWN)
			unknown_count++;
	}
	std::cout << "sat_count " << sat_count << std::endl;
	std::cout << "unsat_count " << unsat_count << std::endl;
	std::cout << "unknown_count " << unknown_count << std::endl;
	MPI_Abort(MPI_COMM_WORLD, 0);
	MPI_Finalize();
#endif
	return true;
}

void SendString(string string_to_send, int computing_process)
{
#ifdef _MPI
	int char_arr_len = string_to_send.size();
	char *char_arr;

	std::cout << "Sending char_arr_len " << char_arr_len << std::endl;
	MPI_Send(&char_arr_len, 1, MPI_INT, computing_process, 0, MPI_COMM_WORLD);
	char_arr = new char[char_arr_len + 1];
	char_arr[char_arr_len] = NULL;
	for (unsigned j = 0; j < char_arr_len; j++)
		char_arr[j] = string_to_send[j];
	std::cout << "Sending char_arr " << char_arr << std::endl;
	MPI_Send(char_arr, char_arr_len + 1, MPI_CHAR, computing_process, 0, MPI_COMM_WORLD);
	delete[] char_arr;
#endif
}

bool computingProcess(int rank, string solvers_dir, string cnfs_dir, double maxtime_seconds)
{
#ifdef _MPI
	MPI_Status status;
	int process_task_index;
	double process_solving_time = 0;
	std::string cnf_instance_name;
	int cnf_name_char_arr_len = 0;
	char *cnf_name_char_arr;
	int solver_name_char_arr_len = 0;
	char *solver_name_char_arr;
	int first_message;
	
	// get maxtime_seconds once
	MPI_Recv(&first_message, 1, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	if (rank == 1)
		cout << "Recevied maxtime_seconds " << maxtime_seconds << std::endl;
	
	// sleep and let other process from the node to launch multithread solver
	if (first_message == SLEEP_MESSAGE) {
		cout << "computing process " << rank << " starts to sleep" << endl;
		sleep(604800);
	}
	
	int result;
	
	for (;;) {
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
		if (rank == 1)
			cout << "Received cnf_name_str " << cnf_name_str << endl;
		if (rank == 1)
			cout << "Received solver_name_str " << solver_name_str << endl;
		
		process_solving_time = MPI_Wtime();
		// solving with received Tfact
		result = callMultithreadSolver(rank, maxtime_seconds, solvers_dir, cnfs_dir, solver_name_str, cnf_name_str);
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
	                       string cnfs_dir, 
	                       string solver_name_str, 
	                       string cnf_name_str)
{
	string maxtime_seconds_str;
	stringstream sstream;
	sstream << maxtime_seconds;
	maxtime_seconds_str = sstream.str();
	sstream.str(""); sstream.clear();
	sstream << CORES_PER_NODE;
	string nof_threads_str = sstream.str();
	sstream.str(""); sstream.clear();
	int result = solveInstance(solvers_dir, cnfs_dir, solver_name_str, cnf_name_str, maxtime_seconds_str, nof_threads_str);
	return result;
}

std::string get_pre_cnf_solver_params_str(std::string solvers_dir, std::string solver_name,
	std::string maxtime_seconds_str, std::string nof_threads_str)
{
	std::string solver_params_str;
	std::string result_str;
	//bool isTimeLimit = false;

	/*if ( ( (solver_name.find("minisat") != std::string::npos) ||
		   (solver_name.find("rokk") != std::string::npos) ) &&
		   (solver_name.find("cryptominisat") == std::string::npos) )
	{
		//std::cout << "minisat_simp detected" << std::endl;
		solver_params_str = "-cpu-lim=";
		isTimeLimit = true;
	}
	else if (solver_name.find("lingeling") != std::string::npos) {
		solver_params_str = "-T ";
		isTimeLimit = true;
	}*/
	
	if (solver_name.find("dimetheus") != std::string::npos)
		solver_params_str += " -formula";
	else if (solver_name.find("cvc4") != std::string::npos)
		solver_params_str += " --smtlib-strict";
	else if (solver_name.find("z3") != std::string::npos)
		solver_params_str += " -smt";
	else if (solver_name.find("Spear") != std::string::npos)
		solver_params_str += " --dimacs";

	//if (isTimeLimit)
	//	solver_params_str += maxtime_seconds_str;
	
	if ((solver_name.find("plingeling") != std::string::npos) ||
		(solver_name.find("treengeling") != std::string::npos))
		solver_params_str += " -t " + nof_threads_str;
	else if (solver_name.find("cryptominisat_parallel") != std::string::npos)
		solver_params_str += "--threads=" + nof_threads_str;

#ifdef _MPI
	string cur_path = Addit_func::exec("echo $PWD");
	cur_path.erase(std::remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(std::remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	result_str = cur_path + "/timelimit -t " + maxtime_seconds_str + " -T 1 " + cur_path + "/" +
		solvers_dir + "/" + solver_name;
#else
	result_str = "./timelimit -t " + maxtime_seconds_str + " -T 1 " + "./" +
		solvers_dir + "/" + solver_name;
#endif

	if (solver_params_str != "")
		result_str += solver_params_str;
    
	return result_str;
}

std::string get_post_cnf_solver_params_str(std::string solver_name)
{
	std::string result_str;
	if ( (solver_name.find("CSCC") != std::string::npos)   || 
	     (solver_name.find("DCCAlm") != std::string::npos) ||
		 (solver_name.find("DCCASat") != std::string::npos)
		)
	{
		result_str = " 1";
	}
	else if (solver_name.find("dimetheus") != std::string::npos) {
		result_str = " -guide 6";
	}
	return result_str;
}

bool isSkipUnusefulSolver(std::string solver_name)
{
	if (solver_name.find("WalkSATlm2013") != std::string::npos)
		return true;

	return false;
}