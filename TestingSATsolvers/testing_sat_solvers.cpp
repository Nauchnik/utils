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

struct solver
{
	string name;
	string base_name;
	double avg_time;
	double min_time;
	double max_time;
	vector<double> svm_parameter_values;
};

struct task
{
	solver s;
	string cnf_name;
	double solving_time;
	int result;
};

struct svm_parameter
{
	string name;
	vector<double> values;
};

string solvers_dir = "";
string instances_dir = "";
string nof_threads_str = "";
string maxtime_seconds_str = "600";
string max_memory_mb_str = "4096";
string svm_pcs_name = "";
string smac_name = "";
// ALIAS params
string alias_pcs_name = "";
int alias_opt_alg = 0;
string alias_backdoor_name = "";
bool is_alias_solve = false;
bool isAlias = false;
//
bool isSvm = false;
bool isSmac = false;
vector<svm_parameter> svm_parameters;

bool conseqProcessing();
int solveInstance(const string solver_name, const string cnf_name);
void solveSmacInstance(const string solver_name, const string scenario_name);
int solveAliasInstance(string solver_name, string cnf_name);
int solveSvmInstance(const string solver_base_name, const string solver_full_name, const string cnf_name, const vector<double> svm_parameters_values);
string get_pre_cnf_solver_params_str(const string solver_name);
string get_post_cnf_solver_params_str(string solver_name);
bool controlProcess(const int corecount);
void sendTask(const int task_index, const int computing_process, const task cur_task);
void sendString(const string string_to_send, const int computing_process);
bool computingProcess(const int rank);
int callMultithreadSolver(const int rank, const string solver_base_name, const string solver_name, 
	const string cnf_name, const vector<double> svm_parameters_values);
void makeSvmParameters();
int getResultFromFile(string out_name);

int main( int argc, char **argv )
{
#ifdef _DEBUG
	svm_pcs_name = "svm.pcs";
	vector<string> solver_files_names;
	solver_files_names.push_back("glucose");
	isSvm = true;

	string solvers_dir1 = "";
	string str_to_remove1 = "./";
	unsigned pos1 = solvers_dir1.find(str_to_remove1);
	if (pos1 != string::npos)
		solvers_dir.erase(pos1, str_to_remove1.length());

	vector<vector<double>> search_space_svm_parameters;
	makeSvmParameters();

	argc = 5;
	argv[1] = "solvers";
	argv[2] = "cnfs";
	argv[3] = "600";
	argv[4] = "4096";
#endif
	
	if ( argc < 3 ) {
		cout << "Usage: prog -solvers_dir [path] -instances_dir [path] -maxseconds [seconds] " <<
			    "-maxthreads [threads] -maxmb [mb] -alias-pcs [pcs_name] -alias-opt-alg [0..5] -alias-backdoor [backdoor_name] --alias-solve " << 
			    "-svm [svm_name] -smac [smac_name]" << endl;
		return 1;
	}

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
		if (str == "-maxthreads") {
			if (i < argc - 1)
				nof_threads_str = argv[i + 1];
		}
		if (str == "-maxseconds") {
			if (i < argc - 1)
				maxtime_seconds_str = argv[i + 1];
		}
		if (str == "-maxmb") {
			if (i < argc - 1)
				max_memory_mb_str = argv[i + 1];
		}
		if (str == "-svm") {
			isSvm = true;
			if (i < argc - 1)
				svm_pcs_name = argv[i + 1];
		}
		if (str == "-smac") {
			isSmac = true;
			if (i < argc - 1)
				smac_name = argv[i + 1];
		}
		if (str == "-alias-pcs") {
			isAlias = true;
			if (i < argc - 1)
				alias_pcs_name = argv[i + 1];
		}
		if (str == "-alias-opt-alg") {
			if (i < argc - 1)
				alias_opt_alg = atoi(argv[i + 1]);
		}
		if (str == "-alias-backdoor") {
			if (i < argc - 1) {
				alias_backdoor_name = argv[i + 1];
				is_alias_solve = true;
			}
		}
		if (str == "--alias-solve") {
			is_alias_solve = true;
		}
	}
	
	string str_to_remove = "./";
	unsigned pos = 0; 
	if (solvers_dir != "") {
		solvers_dir.find(str_to_remove);
		if (pos != string::npos)
			solvers_dir.erase(pos, str_to_remove.length());
	}
	pos = instances_dir.find(str_to_remove);
	if (pos != string::npos)
		instances_dir.erase(pos, str_to_remove.length());

#ifdef _MPI
	cout << "MPI mode " << endl;
	int rank, corecount;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &corecount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (isSvm)
		makeSvmParameters();

	if (rank == 0) {
		cout << "corecount " << corecount << endl;
		cout << "solvers_dir " << solvers_dir << endl;
		cout << "instances_dir " << instances_dir << endl;
		cout << "maxtime_seconds_str " << maxtime_seconds_str << endl;
		cout << "max_memory_mb_str " << max_memory_mb_str << endl;
		cout << "nof_threads_str " << nof_threads_str << endl;
		if (alias_pcs_name != "") {
			cout << "ALIAS mode" << endl;
			cout << "alias_pcs_name " << alias_pcs_name << endl;
			cout << "alias_opt_als " << alias_opt_alg << endl;
			cout << "is_alias_solve " << is_alias_solve << endl;
			cout << "alias_backdoor_name " << alias_backdoor_name << endl;
		}
		if (svm_pcs_name != "") {
			cout << "SVM mode" << endl;
			cout << "svm_pcs_name " << svm_pcs_name << endl;
		}
		if (smac_name != "") {
			cout << "SMAC mode \n";
			cout << "smac_name " << smac_name << endl;
		}
		controlProcess(corecount);
	}
	else
		computingProcess(rank);
#else
	cout << "Conseq mode " << endl;
	cout << "solvers_dir " << solvers_dir << endl;
	cout << "instances_dir " << instances_dir << endl;
	cout << "maxtime_seconds " << maxtime_seconds_str << endl;
	conseqProcessing();
#endif
	
	return 0;
}

bool controlProcess(const int corecount)
{
#ifdef _MPI
	cout << "control MPI process" << endl;
	cout << "*** Stage 1. Collect hosts names." << endl;

	int buffer_size = 0;
	char *buffer;
	MPI_Status status;
	vector<string> hosts_names_vec;
	hosts_names_vec.resize(corecount - 1);
	for (int i = 1; i < corecount; i++) {
		MPI_Probe(i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_CHAR, &buffer_size);
		buffer = new char[buffer_size];
		MPI_Recv(buffer, buffer_size, MPI_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		hosts_names_vec[i - 1] = buffer;
		delete[] buffer;
	}

	cout << "hosts_names : " << endl;
	for (auto &x : hosts_names_vec)
		cout << x << endl;

	/*vector<string> checked_hosts_names;
	vector<int> computing_process_vec; // the vector of processes which have to launch multithread solvers
	for (unsigned i = 0; i < hosts_names_vec.size(); i++)
		if (find(checked_hosts_names.begin(), checked_hosts_names.end(), hosts_names_vec[i]) == checked_hosts_names.end()) {
			checked_hosts_names.push_back(hosts_names_vec[i]);
			computing_process_vec.push_back(i + 1);
		}
	*/

	vector<int> computing_process_vec;
	for (unsigned i = 0; i < hosts_names_vec.size(); i++)
		computing_process_vec.push_back(i + 1);
	
	cout << "computing_process_vec " << endl;
	for (auto &x : computing_process_vec)
		cout << x << " ";
	cout << endl;
	unsigned computing_processes = computing_process_vec.size();

	cout << "*** Stage 2. Solve all instances." << endl;

	string system_str, cur_process_dir_name;
	double control_process_solving_time = MPI_Wtime();
	vector<string> solver_files_names = vector<string>();;
	vector<string> cnf_files_names = vector<string>();
	vector<string> solved_instances;
	string cur_path = exec("echo $PWD");
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	if (isSmac) {
		solvers_dir = "";
		solver_files_names.push_back(smac_name);
	} else {
		if (!getdir(solvers_dir, solver_files_names)) { return false; }
	}
	solvers_dir = cur_path + "/" + solvers_dir;
	cout << "full solvers_dir " << solvers_dir << endl;
	instances_dir = cur_path + "/" + instances_dir;
	cout << "full instances_dir " << instances_dir << endl;
	if (!getdir(instances_dir, cnf_files_names)) { return false; };
	sort(solver_files_names.begin(), solver_files_names.end());
	sort(cnf_files_names.begin(), cnf_files_names.end());

	vector<string> tmp_names;
	for (auto &x : solver_files_names) {
		if (x != "svmsat")
			tmp_names.push_back(x);
	}
	solver_files_names = tmp_names;

	cout << endl << "solver_files_names :" << endl;
	for (vector<string> ::iterator it = solver_files_names.begin(); it != solver_files_names.end(); it++)
		cout << *it << endl;
	cout << endl << "cnf_files_names :" << endl;
	for (vector<string> ::iterator it = cnf_files_names.begin(); it != cnf_files_names.end(); it++)
		cout << *it << endl;

	vector<vector<double>> search_space_svm_parameters;
	if (isSvm) {
		vector<vector<double>> parameters_values_vec;
		for (auto &x : svm_parameters)
			parameters_values_vec.push_back(x.values);
		vector<int> index_arr;
		vector<double> cur_values_vec;
		while (next_cartesian(parameters_values_vec, index_arr, cur_values_vec))
			search_space_svm_parameters.push_back(cur_values_vec);
	}

	vector<solver> solver_vec;
	for (auto &x : solver_files_names) {
		solver s;
		s.name = s.base_name = x;
		s.avg_time = s.max_time = s.min_time = 0;
		s.svm_parameter_values.resize(svm_parameters.size());
		for (auto &y : s.svm_parameter_values)
			y = -1;
		solver_vec.push_back(s);
		if (isSvm) {
			for (auto &y : search_space_svm_parameters) {
				string name = x;
				for (auto &z : y)
					name += "_" + doubletostr(z);
				solver s;
				s.name = name;
				s.base_name = x;
				s.avg_time = s.max_time = s.min_time = 0;
				s.svm_parameter_values = y;
				solver_vec.push_back(s);
			}
		}
	}
	cout << "solver_vec size " << solver_vec.size() << endl;
	cout << "the first 10 names : \n";
	for (unsigned i = 0; i < solver_vec.size(); i++) {
		if (i == 10)
			break;
		cout << solver_vec[i].name << endl;
	}
	
	task cur_task;
	vector<task> tasks_vec;
	for (auto &x : solver_vec)
		for (auto &y : cnf_files_names) {
			cur_task.s = x;
			cur_task.cnf_name = y;
			cur_task.result = UNKNOWN;
			cur_task.solving_time = -1;
			tasks_vec.push_back(cur_task);
		}

	cout << "tasks_vec.size() " << tasks_vec.size() << endl;

	double maxtime_seconds;
	istringstream(maxtime_seconds_str) >> maxtime_seconds;
	int sent_tasks = 0, solved_tasks = 0;
	int first_send_tasks = (computing_processes < tasks_vec.size()) ? computing_processes : tasks_vec.size();
	cout << "first_send_tasks " << first_send_tasks << endl;

	// send data once for 1 process per node
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
	for (int i = 0; i < first_send_tasks; i++)
		sendTask(i, computing_process_vec[i], tasks_vec[i]);
	sent_tasks = first_send_tasks;

	double process_solving_time;
	int process_task_index;
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
		tasks_vec[process_task_index].result = result;
		tasks_vec[process_task_index].solving_time = process_solving_time;
		if (process_solving_time >= maxtime_seconds)
			interrupted++;
		solved_tasks++;
		cout << "solved_tasks " << solved_tasks << endl;
		if (interrupted)
			cout << "interrupted " << solved_tasks << endl;

		if (sent_tasks < tasks_vec.size()) {
			sendTask(sent_tasks, status.MPI_SOURCE, tasks_vec[sent_tasks]);
			sent_tasks++;
			cout << "sent_tasks " << sent_tasks << endl;
		}
		//else {
		// no new tasks, tell computing process to free its resources
		//	MPI_Send(&STOP_MESSAGE, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		//	cout << "stop-message has been sent to computing process " << status.MPI_SOURCE << endl;
		//}  send stop-message on all processes including sleeping ones

		ofile.open("testing_sat_solvers_out", ios_base::out);
		ofile << "solver instance time" << endl;
		for (int i = 0; i < tasks_vec.size(); i++)
			ofile << tasks_vec[i].s.name << " "
			      << tasks_vec[i].cnf_name << " "
			      << tasks_vec[i].solving_time << endl;
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
	for (auto &x : tasks_vec) {
		if (x.solving_time == -1)
			continue;
		sum += x.solving_time;
		min = x.solving_time < min ? x.solving_time : min;
		max = x.solving_time > max ? x.solving_time : max;
	}
	med = sum / tasks_vec.size();
	cout << "min time " << min << endl;
	cout << "max time " << max << endl;
	cout << "med time " << med << endl;
	unsigned sat_count = 0, unsat_count = 0, unknown_count = 0;
	for (auto &x : tasks_vec) {
		if (x.result == SAT)
			sat_count++;
		else if (x.result == UNSAT)
			unsat_count++;
		else if (x.result == UNKNOWN)
			unknown_count++;
	}
	cout << "sat_count " << sat_count << endl;
	cout << "unsat_count " << unsat_count << endl;
	cout << "unknown_count " << unknown_count << endl;

	MPI_Finalize();
#endif
	return true;
}

void makeSvmParameters()
{
	// parse paremeters and their values
	ifstream ifile(svm_pcs_name.c_str());
	string str, parameter_name, parameter_values_str;
	while (getline(ifile, str)) {
		svm_parameter svm_p;
		stringstream sstream;
		sstream << str;
		sstream >> svm_p.name;
		sstream >> parameter_values_str;
		for (auto &x : parameter_values_str)
			if ((x == '{') || (x == '}') || (x == ','))
				x = ' ';
		sstream.clear(); sstream.str("");
		sstream << parameter_values_str;
		double dval;
		while (sstream >> dval)
			svm_p.values.push_back(dval);
		svm_parameters.push_back(svm_p);
	}
	cout << "svm_parameters size " << svm_parameters.size() << endl;
	ifile.close();
}

void sendTask(const int task_index, const int computing_process, const task cur_task)
{
#ifdef _MPI
	cout << "Sending task " << task_index << endl;
	MPI_Send(&task_index, 1, MPI_INT, computing_process, 0, MPI_COMM_WORLD);
	cout << "Sending solver base name " << cur_task.s.base_name << endl;
	sendString(cur_task.s.base_name, computing_process);
	cout << "Sending solver full name " << cur_task.s.name << endl;
	sendString(cur_task.s.name, computing_process);
	cout << "Sending cnf_name " << cur_task.cnf_name << endl;
	sendString(cur_task.cnf_name, computing_process);
	if (isSvm) {
		cout << "sending " << cur_task.s.svm_parameter_values.size() << " parameters values : \n";
		for (auto &x : cur_task.s.svm_parameter_values)
			cout << x << " ";
		cout << endl;
		double *arr = new double[cur_task.s.svm_parameter_values.size()];
		for (unsigned i = 0; i < cur_task.s.svm_parameter_values.size(); i++)
			arr[i] = cur_task.s.svm_parameter_values[i];
		MPI_Send(arr, cur_task.s.svm_parameter_values.size(), MPI_DOUBLE, computing_process, 0, MPI_COMM_WORLD);
		delete[] arr;
	}
#endif
}

bool computingProcess(const int rank)
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
	int solver_base_name_char_arr_len = 0;
	char *solver_name_char_arr;
	char *solver_base_name_char_arr;
	int first_message;

	unsigned nof_threads = 0;
	double maxtime_seconds = 0;
	double max_memory_mb = 0;
	istringstream(nof_threads_str) >> nof_threads;
	istringstream(maxtime_seconds_str) >> maxtime_seconds;
	istringstream(max_memory_mb_str) >> max_memory_mb;

	// get maxtime_seconds once
	MPI_Recv(&first_message, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

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
				//cout << "Received stop message on computing process " << rank << endl;
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
		MPI_Recv(&solver_base_name_char_arr_len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (rank == 1)
			cout << "solver_base_name_char_arr_len " << solver_base_name_char_arr_len << endl;
		solver_base_name_char_arr = new char[solver_base_name_char_arr_len + 1];
		MPI_Recv(solver_base_name_char_arr, solver_base_name_char_arr_len + 1,
			MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		string solver_base_name = solver_base_name_char_arr;
		delete[] solver_base_name_char_arr;
		MPI_Recv(&solver_name_char_arr_len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (rank == 1)
			cout << "solver_name_char_arr_len " << solver_name_char_arr_len << endl;
		solver_name_char_arr = new char[solver_name_char_arr_len + 1];
		MPI_Recv(solver_name_char_arr, solver_name_char_arr_len + 1,
			MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		string solver_name = solver_name_char_arr;
		delete[] solver_name_char_arr;
		MPI_Recv(&cnf_name_char_arr_len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if (rank == 1)
			cout << "cnf_name_char_arr_len " << cnf_name_char_arr_len << endl;
		cnf_name_char_arr = new char[cnf_name_char_arr_len + 1];
		MPI_Recv(cnf_name_char_arr, cnf_name_char_arr_len + 1,
			MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		string cnf_name = cnf_name_char_arr;
		delete[] cnf_name_char_arr;
		if (rank == 1) {
			cout << "Received solver_base_name " << solver_base_name << endl;
			cout << "Received solver_name " << solver_name << endl;
			cout << "Received cnf_name " << cnf_name << endl;
		}

		vector<double> svm_parameters_values;
		if (isSvm) {
			cout << "recv svm parameters values \n";
			cout << "svm_parameters size " << svm_parameters.size() << endl;
			double *svm_parameters_arr = new double[svm_parameters.size()];
			MPI_Recv(svm_parameters_arr, svm_parameters.size(), MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			cout << "recv done \n";
			for (unsigned i = 0; i < svm_parameters.size(); i++) {
				svm_parameters_values.push_back(svm_parameters_arr[i]);
				cout << svm_parameters_values[i] << " ";
			}
			delete[] svm_parameters_arr;
		}
		
		process_solving_time = MPI_Wtime();
		// solving with received Tfact
		result = callMultithreadSolver(rank, solver_base_name, solver_name, cnf_name, svm_parameters_values);
		process_solving_time = MPI_Wtime() - process_solving_time;
		/*if (result == UNKNOWN) {
			cout << "unknown result, set huge solving time value \n";
			process_solving_time = 1e100;
		}*/
		if (rank == 1) {
			cout << "rank 1\n";
			cout << "sending process_task_index " << process_task_index << endl;
			cout << "sending " << process_solving_time << endl;
			cout << "result " << result << endl;
		}
		
		MPI_Send(&process_task_index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&process_solving_time, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
#endif
	return true;
}

int callMultithreadSolver(const int rank, const string solver_base_name, const string solver_name,
						  const string cnf_name, const vector<double> svm_parameters_values)
{
	if ((solver_name == "") || (cnf_name == ""))
		return 0;

	if (nof_threads_str == "") {
		int cores_per_node = thread::hardware_concurrency();
		stringstream sstream;
		sstream << cores_per_node;
		nof_threads_str = sstream.str();
		sstream.str(""); sstream.clear();
		cout << "new nof_threads_str " << nof_threads_str << endl;
	}

	int result = 0;

	if (isAlias)
		result = solveAliasInstance(solver_name, cnf_name);
	else if ( (isSvm) && (solver_base_name != solver_name) )
		result = solveSvmInstance(solver_base_name, solver_name, cnf_name, svm_parameters_values);
	else if (isSmac)
		solveSmacInstance(solver_name, cnf_name);
	else
		result = solveInstance(solver_name, cnf_name);

	return result;
}

string get_pre_cnf_solver_params_str(const string solver_name)
{
	string solver_params_str;
	/*if ( (solver_name.find(".sh") == string::npos) &&
		 (solver_name.find("plingeling") == string::npos) && 
		 (solver_name.find("treengeling") == string::npos) &&
		 (solver_name.find("lingeling") == string::npos)
	   )
		solver_params_str = "-cpu-lim=" + maxtime_seconds_str + " ";*/
	
	if ((solver_name.find("plingeling") != string::npos) ||
		(solver_name.find("treengeling") != string::npos))
		solver_params_str += "-t " + nof_threads_str;
	else if (solver_name.find("cryptominisat") != string::npos)
		solver_params_str += "--threads=" + nof_threads_str;
	else if (solver_name.find("syrup") != string::npos)
		solver_params_str += "-nthreads=" + nof_threads_str;
	else if ((solver_name.find("hordesat") != string::npos) ||
		(solver_name.find("painless") != string::npos))
		solver_params_str += "-c=" + nof_threads_str;
	else if (solver_name.find("abcdsat_p") != string::npos)
		solver_params_str += "-maxnbthreads=" + nof_threads_str;
	else if (solver_name.find("cvc4") != string::npos)
		solver_params_str += "--lang smtlib2";
	else if (solver_name.find("z3") != string::npos)
		solver_params_str += "-smt2";

	double max_memory_mb;
	istringstream(max_memory_mb_str) >> max_memory_mb;
	double max_memory_gb = max_memory_mb / 1024;
	stringstream sstream;
	sstream << max_memory_gb;
	string max_memory_gb_str = sstream.str();
	if (solver_name.find("painless") != string::npos)
		solver_params_str += " -max-memory=" + max_memory_gb_str;
	else if (solver_name.find("syrup") != string::npos)
		solver_params_str += " -maxmemory=" + max_memory_mb_str;
	else if ((solver_name.find("plingeling") != string::npos) ||
		(solver_name.find("treengeling") != string::npos))
		solver_params_str += " -m " + max_memory_mb_str;

	string result_str;
#ifdef _MPI
	string cur_path = exec("echo $PWD");
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	result_str = cur_path + "/timelimit -t " + maxtime_seconds_str + " -T 1 " + cur_path + "/" +
		solvers_dir + "/" + solver_name + " " + solver_params_str;
#else
	result_str = "./timelimit -t " + maxtime_seconds_str + " -T 1 " + "./" +
		solvers_dir + "/" + solver_name;
#endif

	return result_str;
}

string get_post_cnf_solver_params_str(const string solver_name)
{
	string result_str;
	if ((solver_name.find("CSCC") != string::npos) ||
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

void sendString(const string string_to_send, const int computing_process)
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

bool conseqProcessing()
{
	/*fstream current_out;
	double cur_time, avg_time = 0;
	stringstream sstream;
	
	if (nof_threads_str == "") {
		unsigned int nthreads = thread::hardware_concurrency();
		cout << "nthreads " << nthreads << endl;
		sstream << nthreads;
		nof_threads_str = sstream.str();
		sstream.clear(); sstream.str("");
		cout << "nof_threads_str " << nof_threads_str << endl;
	}

	vector<string> solver_files_names = vector<string>();
	vector<string> cnf_files_names = vector<string>();

	if (!getdir(solvers_dir, solver_files_names)) { return false; }
	if (!getdir(instances_dir, cnf_files_names)) { return false; };
	sort(solver_files_names.begin(), solver_files_names.end());
	sort(cnf_files_names.begin(), cnf_files_names.end());

	cout << endl << "solver_files_names :" << endl;
	for (vector<string> ::iterator it = solver_files_names.begin(); it != solver_files_names.end(); it++)
		cout << *it << endl;
	cout << endl << "cnf_files_names :" << endl;
	for (vector<string> ::iterator it = cnf_files_names.begin(); it != cnf_files_names.end(); it++)
		cout << *it << endl;

	vector<solver> solver_vec;
	solver cur_solver_info;
	vector<unsigned> sat_count_vec;
	sat_count_vec.resize(solver_files_names.size());
	for (vector<unsigned> ::iterator it = sat_count_vec.begin(); it != sat_count_vec.end(); it++)
		*it = 0;

	unsigned solved_problems_count;
	vector< vector<string> > solver_cnf_times_str;
	string solver_time_str;
	stringstream convert_sstream;
	solver_cnf_times_str.resize(solver_files_names.size());
	double maxtime_seconds;
	istringstream(maxtime_seconds_str) >> maxtime_seconds;
	for (unsigned i = 0; i < solver_files_names.size(); i++) {
		double sum_time = 0, min_time = 0, max_time = 0;
		solved_problems_count = 0;
		for (unsigned j = 0; j < cnf_files_names.size(); j++) {
			chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
			int result = solveInstance(solver_files_names[i], cnf_files_names[j]);
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
		solver_vec.push_back(cur_solver_info);

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
	*/

	return true;
}

int solveAliasInstance(const string solver_name, const string cnf_name)
{
	cout << "start solveAliasInstance()" << endl;
	string base_path = exec("echo $PWD");
	base_path.erase(remove(base_path.begin(), base_path.end(), '\r'), base_path.end());
	base_path.erase(remove(base_path.begin(), base_path.end(), '\n'), base_path.end());
	cout << "base path " << base_path << endl;
	
	// launch the script to create a folder for ALIAS and copy files to it
	string system_str = base_path + "/alias_prepare_dir.sh " + 
		 solvers_dir + "/" + solver_name + " " +
		 instances_dir + "/" + cnf_name;
	if (alias_pcs_name != "")
		system_str += " " + alias_pcs_name;
	cout << "alias_prepare_dir.sh command string " << system_str << endl;

	string result_str = exec(system_str);
	cout << "result_str " << result_str << endl;
	
	string alias_launch_path = base_path + "/tmp_" + solver_name + "_" + cnf_name;
	system_str = alias_launch_path + "/alias_ls" +
		" -solver=" + alias_launch_path + "/" + solver_name +
		" -script=" + alias_launch_path + "/ALIAS.py" +
		" -cpu-lim=" + maxtime_seconds_str +
		" -verb=1" +
		" " + alias_launch_path + "/" + cnf_name;
	if (alias_pcs_name != "")
		system_str += " -pcs=" + alias_launch_path + "/" + alias_pcs_name;
	if (is_alias_solve)
		system_str += " --solve";
	if (alias_backdoor_name != "")
		system_str += " -backdoor=" + alias_backdoor_name;
	else
		system_str += " -opt-alg=" + to_string((long long)alias_opt_alg);
	
	cout << "alias_ls command string " << system_str << endl;
	
	string out_name = base_path + "/out_" + solver_name + "_" + cnf_name;
	fstream out_file;
	out_file.open(out_name, ios_base::out);
	out_file << exec(system_str);
	out_file.close();
	out_file.clear();
	
	return UNKNOWN;
}

int solveSvmInstance(const string solver_base_name, const string solver_full_name, const string cnf_name, const vector<double> svm_parameters_values)
{
	cout << "start solveSvmInstance()" << endl;
	string base_path = exec("echo $PWD");
	base_path.erase(remove(base_path.begin(), base_path.end(), '\r'), base_path.end());
	base_path.erase(remove(base_path.begin(), base_path.end(), '\n'), base_path.end());
	cout << "base path " << base_path << endl;

	// launch the script to create a folder for ALIAS and copy files to it
	string system_str = base_path + "/svm_prepare_dir.sh " +
		solvers_dir + "/" + solver_base_name + " " +
		solver_full_name + " " +
		instances_dir + "/" + cnf_name;
	cout << "svm_prepare_dir.sh command string " << system_str << endl;

	string result_str = exec(system_str);
	cout << "result_str " << result_str << endl;

	string svm_launch_path = base_path + "/tmp_" + solver_full_name + "_" + cnf_name + "/";
	system_str = svm_launch_path + "svmsat " +
		svm_launch_path + solver_base_name + " " +
		svm_launch_path + cnf_name +
		" -cpu-lim=" + maxtime_seconds_str;
	if (svm_parameters_values.size() == svm_parameters.size())
		for (unsigned i = 0; i < svm_parameters.size(); i++) {
			if (svm_parameters_values[i] != -1)
				system_str += " -" + svm_parameters[i].name + "=" + doubletostr(svm_parameters_values[i]);
		}
	cout << "svmsat command string " << system_str << endl;

	string out_name = base_path + "/out_" + solver_full_name + "_" + cnf_name;
	fstream out_file;
	out_file.open(out_name, ios_base::out);
	out_file << exec(system_str);
	out_file.close();
	out_file.clear();

	// get result from the out file
	int result = getResultFromFile(out_name);

	cout << "remove temp folder " << svm_launch_path << endl;
	system_str = "rm -rf " + svm_launch_path;
	exec(system_str);
	
	return result;
}

void solveSmacInstance(const string solver_name, const string scenario_name)
{
	cout << "start solveSmacInstance() \n";
	string cur_path = exec("echo $PWD");
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	cout << "cur_path " << cur_path << endl;
	string system_str = cur_path + "/" + solver_name + " " + cur_path + "/" + instances_dir +
		"/" + scenario_name + " out_" + solver_name + "_" + scenario_name;
	cout << "system_str " << system_str << endl;
	exec(system_str);
}

int solveInstance(const string solver_name, const string cnf_name)
{
	string system_str, current_out_name, str;
	
#ifdef _MPI
	string cur_path = exec("echo $PWD");
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\r'), cur_path.end());
	cur_path.erase(remove(cur_path.begin(), cur_path.end(), '\n'), cur_path.end());
	current_out_name = cur_path + "/out_" + solver_name + "_" + cnf_name;
	system_str = get_pre_cnf_solver_params_str(solver_name) + " " + cur_path + 
		"/" + instances_dir + "/" + cnf_name + get_post_cnf_solver_params_str(solver_name);
#else
	current_out_name = "out_" + solver_name + "_" + cnf_name;
	system_str = get_pre_cnf_solver_params_str(solver_name) + " ./" + instances_dir + 
		"/" + cnf_name + get_post_cnf_solver_params_str(solver_name);
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
	current_out << exec(system_str);
	current_out.clear();
	current_out.close();

	int result = getResultFromFile(current_out_name);

	return result;
}

int getResultFromFile(string out_name)
{
	ifstream out_file(out_name.c_str());
	string str;
	int result = UNKNOWN;
	while (getline(out_file, str)) {
		if ((str.find("s SATISFIABLE") != string::npos) || (str.find("SATISFIABLE") == 0)) {
			result = SAT;
			break;
		}
		else if ((str.find("s UNSATISFIABLE") != string::npos) || (str.find("UNSATISFIABLE") == 0)) {
			result = UNSAT;
			break;
		}
	}
	out_file.close();
	return result;
}