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
#include "addit_func.h"

std::string basic_launch_dir = "/home/ozaikin/2016-02_Ulyantsev/80vars_MPI_multithread_solver/";
std::string basic_cnf_dir_name = "cnfs";
const unsigned CORES_PER_NODE = 32;

using namespace Addit_func;

struct solver_info
{
	std::string name;
	double avg_time;
	double min_time;
	double max_time;
};

bool conseqProcessing(std::string solvers_dir, std::string cnfs_dir, double maxtime_seconds, 
	                  std::string maxtime_seconds_str);
std::string get_pre_cnf_solver_params_str(std::string solvers_dir, std::string solver_name,
	std::string maxtime_seconds_str, std::string nof_threads_str);
std::string get_post_cnf_solver_params_str(std::string solver_name);
std::vector<std::string> getDataFromSmacValidation();
bool controlProcess(int corecount, std::string solvers_dir, std::string cnfs_dir, double maxtime_seconds);
bool computingProcess(int rank);
void callMultithreadSolver(int rank, std::string cnf_instance_name);

int main( int argc, char **argv )
{
	
#ifdef _DEBUG
	argc = 3;
	argv[1] = "solvers";
	argv[2] = "cnfs";
#endif

	if ( argc < 3 ) {
		std::cout << "Usage: [solvers_path] [cnfs_path] [maxtime_seconds_one_problem]" << std::endl;
		return 1;
	}

	std::string maxtime_seconds_str;
	if ( argc == 3 ) {
		maxtime_seconds_str = "600"; 
		std::cout << "maxtime_seconds was set to default == 600 seconds" << std::endl;
	} else
		maxtime_seconds_str = argv[3];

	double maxtime_seconds; 
	std::istringstream(maxtime_seconds_str) >> maxtime_seconds;

	std::string solvers_dir, cnfs_dir;
	solvers_dir = argv[1];
	cnfs_dir = argv[2];

	std::cout << "solvers_dir " << solvers_dir << std::endl;
	std::cout << "cnfs_dir " << cnfs_dir << std::endl;
	std::cout << "maxtime_seconds " << maxtime_seconds_str << std::endl;

#ifndef _MPI
	conseqProcessing(solvers_dir, cnfs_dir, maxtime_seconds, maxtime_seconds_str);
#else
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &corecount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
		controlProcess(corecount, solvers_dir, cnfs_dir, maxtime_seconds);
	else
		computingProcess(rank);
	parallelProcessing(solvers_dir, cnfs_dir, maxtime_seconds);
#endif
	
	return 0;
}

bool conseqProcessing(std::string solvers_dir, std::string cnfs_dir, double maxtime_seconds, std::string maxtime_seconds_str)
{
	double clock_solving_time;
	std::string system_str, current_out_name, str;
	unsigned copy_from, copy_to;
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

	std::vector<std::string> solver_files_names = std::vector<std::string>();
	std::vector<std::string> cnf_files_names = std::vector<std::string>();

	if (!Addit_func::getdir(solvers_dir, solver_files_names)) { return false; }
	if (!Addit_func::getdir(cnfs_dir, cnf_files_names)) { return false; };
	sort(solver_files_names.begin(), solver_files_names.end());
	sort(cnf_files_names.begin(), cnf_files_names.end());

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

	bool isTimeStr, isSAT;
	unsigned solved_problems_count;
	double sum_time = 0, min_time = 0, max_time = 0;
	std::vector< std::vector<std::string> > solver_cnf_times_str;
	std::string solver_time_str;
	std::stringstream convert_sstream;
	solver_cnf_times_str.resize(solver_files_names.size());
	for (unsigned i = 0; i < solver_files_names.size(); i++) {
		sum_time = 0;
		solved_problems_count = 0;
		for (unsigned j = 0; j < cnf_files_names.size(); j++) {
			current_out_name = "out_" + solver_files_names[i] + "_" + cnf_files_names[j];
			system_str = get_pre_cnf_solver_params_str(solvers_dir, solver_files_names[i], maxtime_seconds_str, nof_threads_str) +
				" ./" + cnfs_dir + "/" + cnf_files_names[j] + get_post_cnf_solver_params_str(solver_files_names[i]);
			std::cout << system_str << std::endl;
			// + " &> ./" + current_out_name;
			//std::cout << system_str << std::endl;
			//cout << "system_result_stream" << endl;
			//cout << system_result_stream.str() << endl;
			//system( system_str.c_str( ) );
			current_out.open(current_out_name.c_str(), std::ios_base::out);
			std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
			current_out << Addit_func::exec(system_str);
			std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			clock_solving_time = time_span.count();
			current_out.close();
			std::cout << "clock_solving_time " << clock_solving_time << std::endl;

			current_out.open(current_out_name.c_str(), std::ios_base::in);

			isSAT = false;
			cur_time = 0.0;
			while (getline(current_out, str)) {
				if (str.find("SATISFIABLE") != std::string::npos) {
					isSAT = true;
					sat_count_vec[i]++;
					std::cout << "SAT found" << std::endl;
					std::cout << "current_out_name " << current_out_name << std::endl;
					std::cout << "sat_count_vec" << std::endl;
					std::cout << solver_files_names[i] << " : " << sat_count_vec[i] << " sat from " <<
						cnf_files_names.size() << std::endl;
				}
				isTimeStr = true;
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

			if (cur_time <= 0.0)
				cur_time = clock_solving_time;
			std::cout << "cur_time " << cur_time << std::endl;

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

		std::string solver_out_file_name = "out_" + solver_files_names[i] + "_total";
		std::ofstream solver_out_file(solver_out_file_name.c_str());
		for (unsigned t = 0; t < solver_cnf_times_str[i].size(); t++)
			solver_out_file << solver_cnf_times_str[i][t] << std::endl;
		solver_out_file.close(); solver_out_file.clear();

		cur_solver_info.name = solver_files_names[i];
		cur_solver_info.avg_time = avg_time;
		cur_solver_info.min_time = min_time;
		cur_solver_info.max_time = max_time;
		solver_info_vec.push_back(cur_solver_info);
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

bool controlProcess(int corecount, std::string solvers_dir, std::string cnfs_dir, double maxtime_seconds)
{
	std::string system_str, cur_process_dir_name;
	std::cout << "Start of the control process" << std::endl;
	
#ifdef _MPI
	double control_process_solving_time = MPI_Wtime();
	unsigned interrupted = 0;
	
	// make list of tasks from SMAC output
	std::vector<std::string> unsolved_instances_names = getDataFromSmacValidation();
	std::cout << "unsolved_instances_names.size() " << unsolved_instances_names.size() << std::endl;
	unsigned tasks = unsolved_instances_names.size();
	std::cout << "tasks_number " << tasks << std::endl;
	int sent_tasks = 0, solved_tasks = 0;
	
	std::vector<int> computing_process_vec; // vector of processes which must launch a multithread solver
	for (unsigned i = 0; i < (corecount / CORES_PER_NODE); i++)
		computing_process_vec.push_back(i*CORES_PER_NODE + 1);
	
	std::cout << "computing_process_vec " << std::endl;
	for (auto &x : computing_process_vec)
		std::cout << x << " ";
	std::cout << std::endl;
	
	int computing_processes = computing_process_vec.size();
	std::cout << "computing_processes " << computing_processes << std::endl;
	
	// send maxtime_seconds data once for 1 process per node
	// for the others processes from a node send the sleep message
	double sleep_message = -1;
	for (int i = 1; i < corecount; i++) {
		if (std::find(computing_process_vec.begin(), computing_process_vec.end(), i) != computing_process_vec.end()))
			MPI_Send(&maxtime_seconds, 1, MPI_DOUBLE, i + 1, 0, MPI_COMM_WORLD); // process for solver launching
		else
			MPI_Send(&sleep_message, 1, MPI_DOUBLE, i + 1, 0, MPI_COMM_WORLD); // process for sleeping
	}
	
	int first_send_tasks = (computing_processes < tasks) ? computing_processes : tasks;
	std::cout << "first_send_tasks " << first_send_tasks << std::endl;
	int cur_cnf_instance_name_char_arr_len = 0;
	char *cur_cnf_instance_name_char_arr;

	// send first part of tasks
	for (int i = 0; i < first_send_tasks; i++) {
		MPI_Send(&i, 1, MPI_INT, computing_process_vec[i], 0, MPI_COMM_WORLD);
		std::cout << "Sending sent_tasks " << i << std::endl;
		cur_cnf_instance_name_char_arr_len = unsolved_instances_names[i].size();
		MPI_Send(&cur_cnf_instance_name_char_arr_len, 1, MPI_INT, computing_process_vec[i], 0, MPI_COMM_WORLD);
		std::cout << "Sending cur_cnf_instance_name_char_arr_len " << cur_cnf_instance_name_char_arr_len << std::endl;
		cur_cnf_instance_name_char_arr = new char[cur_cnf_instance_name_char_arr_len+1];
		cur_cnf_instance_name_char_arr[cur_cnf_instance_name_char_arr_len] = NULL;
		for (unsigend j = 0; j < cur_cnf_instance_name_char_arr_len; j++)
			cur_cnf_instance_name_char_arr[j] = unsolved_instances_names[i][j];
		MPI_Send(cur_cnf_instance_name_char_arr, cur_cnf_instance_name_char_arr_len+1, MPI_CHAR, computing_process_vec[i], 0, MPI_COMM_WORLD);
		std::cout << "Sending cur_cnf_instance_name_char_arr_len " << cur_cnf_instance_name_char_arr << std::endl;
		delete[] cur_cnf_instance_name_char_arr;
	}
	sent_tasks = first_send_tasks;
	
	MPI_Status status;
	double process_func_min, process_solving_time;
	int process_task_index;
	std::vector<double> process_func_min_vec, process_solving_time_vec;
	process_func_min_vec.resize(tasks);
	for (auto &x : process_func_min_vec)
		x = -1;
	process_solving_time_vec.resize(tasks);
	for (auto &x : process_solving_time_vec)
		x = -1;
	std::ofstream ofile;
	
	while (solved_tasks < tasks) {
		MPI_Recv(&process_task_index, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		std::cout << "Recieved process_task_index " << process_task_index << std::endl;
		MPI_Recv(&process_solving_time, 1, MPI_DOUBLE, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		std::cout << "Recieved process_solving_time " << process_solving_time << std::endl;
		process_solving_time_vec[process_task_index] = process_solving_time;
		if (process_solving_time >= max_maxtime_seconds)
			interrupted++;
		solved_tasks++;
		std::cout << "solved_tasks " << solved_tasks << std::endl;
		if (interrupted)
			std::cout << "interrupted " << solved_tasks << std::endl;
		
		if (sent_tasks < tasks) {
			// send a new task if one exists
			MPI_Send(&sent_tasks, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			std::cout << "Sending sent_tasks " << sent_tasks << std::endl;
			cur_cnf_instance_name_char_arr_len = unsolved_instances_names[sent_tasks].size();
			MPI_Send(&cur_cnf_instance_name_char_arr_len, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			std::cout << "Sending cur_cnf_instance_name_char_arr_len " << cur_cnf_instance_name_char_arr_len << std::endl;
			cur_cnf_instance_name_char_arr = new char[cur_cnf_instance_name_char_arr_len + 1];
			cur_cnf_instance_name_char_arr[cur_cnf_instance_name_char_arr_len] = NULL;
			for (unsigend j = 0; j < cur_cnf_instance_name_char_arr_len; j++)
				cur_cnf_instance_name_char_arr[j] = unsolved_instances_names[sent_tasks][j];
			MPI_Send(cur_cnf_instance_name_char_arr, cur_cnf_instance_name_char_arr_len + 1, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			std::cout << "Sending cur_cnf_instance_name_char_arr_len " << cur_cnf_instance_name_char_arr << std::endl;
			delete[] cur_cnf_instance_name_char_arr;
			sent_tasks++;
			std::cout << "sent_tasks " << sent_tasks << std::endl;
		}
		else {
			// no new tasks, tell computing process to free its resources
			MPI_Send(&stop_message, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			std::cout << "stop-message has been sent to computing process " << status.MPI_SOURCE << std::endl;
		}
		
		ofile("testing_sat_solvers_out", std::ios_base::out);
		ofile << "instance time" << std::endl;
		for (int i = 0; i < tasks; i++)
			ofile << unsolved_instances_names[i] << " " << process_solving_time_vec[i] << std::endl;
		ofile.close(); ofile.clear();
	}
	
	std::cout << "End of the control process" << std::endl;
	control_process_solving_time = MPI_Wtime() - control_process_solving_time;
	std::cout << "control_process_solving_time " << control_process_solving_time << std::endl;
	std::cout << "interrupted " << interrupted << " from " << tasks << std::endl;
	
#endif
	return true;
}

bool computingProcess(int rank)
{
#ifdef _MPI
	MPI_Status status;
	int process_task_index;
	double process_solving_time = 0;
	std::string cnf_instance_name;
	double maxtime_seconds = 0;
	int cur_cnf_instance_name_char_arr_len = 0;
	char *cur_cnf_instance_name_char_arr;
	std::string cur_cnf_instance_name_str;
	double sleep_message = -1;

	// get maxtime_seconds once
	MPI_Recv(&maxtime_seconds, 1, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	if (rank == 1)
		std::cout << "Recevied maxtime_seconds " << maxtime_seconds << std::endl;

	// sleep and let other process from the node to launch multithread solver
	if (maxtime_seconds == sleep_message) {
		std::cout << "rank " << rank << " starts sleeping" << std::endl;
		sleep(604800);
	}
	
	for (;;) {
		MPI_Recv(&process_task_index, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (rank == 1)
			std::cout << "Received process_task_index " << process_task_index << std::endl;
		if (process_task_index == stop_message)
			return true;
		MPI_Recv(&cur_cnf_instance_name_char_arr_len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (rank == 1)
			std::cout << "cur_cnf_instance_name_char_arr_len " << cur_cnf_instance_name_char_arr_len << std::endl;
		cur_cnf_instance_name_char_arr = new char[cur_cnf_instance_name_char_arr_len + 1];
		MPI_Recv(cur_cnf_instance_name_char_arr, cur_cnf_instance_name_char_arr_len + 1,
			MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		cur_cnf_instance_name_str = cur_cnf_instance_char_arr;
		delete[] cur_cnf_instance_name_char_arr;
		if (rank == 1)
			std::cout << "Received cur_cnf_instance_name_str " << cur_cnf_instance_name_str << std::endl;

		process_solving_time = MPI_Wtime();
		// solving with received Tfact
		callMultithreadSolver(rank, cur_cnf_instance_name_str);
		process_solving_time = MPI_Wtime() - process_solving_time;

		MPI_Send(&process_task_index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&process_solving_time, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}
#endif
	return true;
}

std::vector<std::string> getDataFromSmacValidation()
{
	std::string smac_validation_file_name = "validationResultsMatrix-tunertime-run0.csv";
	std::ifstream smac_validation_file(smac_validation_file_name.c_str());
	std::vector<std::string> unsolved_instances;

	std::string str;
	int index_from = -1, index_to = -1;
	unsigned value_count;
	std::vector<double> solved_instances_time;
	std::string cur_value_str, instance_name, obj_str, time_str;
	double time_value;
	while (getline(smac_validation_file, str)) {
		if (str.find("Objective") != std::string::npos)
			continue;
		value_count = 0;
		for (unsigned i = 0; i < str.size(); i++) {
			if (str[i] == '\"') {
				if ((i < str.size() - 1) && (index_from == -1))
					index_from = i + 1;
				else if (index_to == -1) {
					index_to = i - 1;
					cur_value_str = str.substr(index_from, index_to - index_from + 1);
					switch (value_count) {
					case 1: instance_name = cur_value_str;
					case 2: obj_str = cur_value_str;
					case 3: time_str = cur_value_str;
					}
					value_count++;
					index_from = -1; // reset indexes
					index_to = -1;
				}
			}
		}
		if (obj_str == time_str) {
			std::istringstream(time_str) >> time_value;
			solved_instances_time.push_back(time_value);
		}
		else {
			for (unsigned j = instance_name.size() - 1; j>0; j--)
				if (instance_name[j] == '/') {
					instance_name = instance_name.substr(j + 1, instance_name.size() - j);
					break;
				}
			unsolved_instances.push_back(instance_name);
		}
	}
	smac_validation_file.close();

	double min = 1e50, max = 0, med = -1, sum = 0;
	for (auto &x : solved_instances_time) {
		if (x < min) min = x;
		if (x > max) max = x;
		sum += x;
	}
	if (solved_instances_time.size() > 0)
		med = sum / solved_instances_time.size();
	return unsolved_instances;
}

void callMultithreadSolver(int rank, std::string cnf_instance_name)
{
	// make script for entering process folder and laucnhing program in it
	std::string system_str;
	std::string full_path_dir_name;
	full_path_dir_name = basic_launch_dir + dir_name + "/";
	std::string basic_launch_dir = "/home/ozaikin/2016-02_Ulyantsev/80vars_MPI_multithread_solver/";
	std::string basic_cnf_dir_name = "cnfs";
	//system_str = "./" + program_name;
	std::cout << "system_str " << system_str << std::endl;
	system(system_str.c_str());
}

std::string get_pre_cnf_solver_params_str(std::string solvers_dir, std::string solver_name,
	std::string maxtime_seconds_str, std::string nof_threads_str)
{
	std::string result_str;
	if ((solver_name.find("minisat//minisat") != std::string::npos) ||
		(solver_name.find("minisat_simp//minisat_simp") != std::string::npos))
	{
		//std::cout << "minisat_simp detected" << std::endl;
		result_str = "-cpu-lim=";
	}
	// glucose can't stop in time
	/*if ( solver_name.find( "glucose" ) != std::string::npos ) {
	std::cout << "glucose detected" << std::endl;
	result_str = "-cpu-lim=";
	}*/
	else if (solver_name.find("sinn") != std::string::npos) {
		//std::cout << "sinn detected" << std::endl;
		result_str = "-cpu-lim=" + maxtime_seconds_str;
	}
	else if (solver_name.find("minisat_bit") != std::string::npos) {
		//std::cout << "minisat_bit detected" << std::endl;
		result_str = "-cpu-lim=";
	}
	else if (solver_name.find("zenn") != std::string::npos) {
		//std::cout << "zenn detected" << std::endl;
		result_str = "-cpu-lim=";
	}
	else if (solver_name.find("glueminisat") != std::string::npos) {
		//std::cout << "glueminisat detected" << std::endl;
		result_str = "-cpu-lim=";
	}
	else if (solver_name.find("minigolf") != std::string::npos) {
		//std::cout << "minigolf detected" << std::endl;
		result_str = "-cpu-lim=";
	}
	/*else if ( solver_name.find( "plingeling" ) != std::string::npos ) {
	//std::cout << "pingeling detected" << std::endl;
	result_str = "-nof_threads ";
	result_str += nof_threads_str;
	result_str += " -t ";
	}
	else if ( solver_name.find( "trengeling" ) != std::string::npos ) {
	//std::cout << "treengeling detected" << std::endl;
	//result_str = "-t " + "11" + nof_threads_str;
	}*/
	else if ((solver_name.find("lingeling") != std::string::npos) &&
		(solver_name.find("plingeling") == std::string::npos)) {
		//std::cout << "lingeling detected" << std::endl;
		result_str = "-t ";
	}
	
	if (result_str == "") {
		std::cout << "unknown solver detected. using timelimit" << std::endl;
		result_str = "./timelimit -t " + maxtime_seconds_str + " -T 1 " + "./" + solvers_dir + "/" + solver_name;
	}
	else
		result_str = "./" + solvers_dir + "/" + solver_name + " " + result_str + maxtime_seconds_str;

	if (solver_name.find("dimetheus") != std::string::npos)
		result_str += " -formula";
	else if (solver_name.find("cvc4") != std::string::npos)
		result_str += " --smtlib-strict";
	else if (solver_name.find("z3") != std::string::npos)
		result_str += " -smt";
	else if (solver_name.find("Spear") != std::string::npos)
		result_str += " --dimacs";
    
	return result_str;
}

std::string get_post_cnf_solver_params_str(std::string solver_name)
{
	std::string result_str;
	if (solver_name.find("CSCC") != std::string::npos) {
		result_str = " 1";
	}
	return result_str;
}