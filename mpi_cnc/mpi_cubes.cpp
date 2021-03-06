#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstring>

#pragma warning(disable : 4996)

const int TIME_BUFFER_SIZE = 80;

using namespace std;

const int NOT_STARTED = -1;
const int IN_PROGRESS = 0;
const int PROCESSED = 1;
const int UNSAT = 2;
const int SAT = 3;
const int INDET = 4;
const int REPORT_EVERY_SEC = 100;
const int CORES_PER_NODE = 36;
const string LOCAL_DIR = "/store/ozaikin/";

struct wu
{
	int id;
	int status;
	int result;
	vector<int> cube;
	double processing_time;
};

bool compareByCubeSize(const wu &a, const wu &b)
{
    return a.cube.size() > b.cube.size();
}

vector<string> files_to_copy{"./iglucose", "./cadical130", "./march_cu", "./timelimit"};

void controlProcess(const int corecount, const string cubes_file_name, const string cube_cpu_lim_str);
vector<wu> readCubes(const string cubes_file_name);
vector<wu> readAndSortCubes(const string cubes_file_name);
void sendWU(vector<wu> &wu_vec, const int wu_id, const int computing_process_id);
void computingProcess(const int rank, const string solver_file_name, const string cnf_file_name, 
					  const string cubes_file_name, const string cube_cpu_lim_str);
void writeInfoOutFile(const string control_process_ofile_name, vector<wu> wu_vec, 
	const double start_time, const string cube_cpu_lim_str);
int getResultFromFile(const string out_name);
void writeProcessingInfo(vector<wu> &wu_vec);
string exec(const string cmd_str);
string intToStr(const int x);
bool hasPrefix(string str, string prefix, string &value);

int total_processed_wus = 0;
int verb = 0;

int main(int argc, char *argv[])
{	
	int rank = 0, corecount = 0;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &corecount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
		cout << "corecount : " << corecount << endl;
	
	if (argc < 5) {
		cerr << "Usage : prog solver-name cnf-file-name cubes-file-name cube-cpu-limit [-verb=0|1]";
		return 1;
	}
	
	string solver_file_name = argv[1];
	string cnf_file_name	= argv[2];
	string cubes_file_name	= argv[3];
	string cube_cpu_lim_str = argv[4];
	if (argc >= 6) {
		string str = argv[5];
		string value;
		if (hasPrefix( str, "-verb=", value ))
			istringstream(value) >> verb;
	}
	
	if (rank == 0)
		cout << "verb : " << verb << endl;

	// make a local directory on each node if it does not exist yet
	// copy to it all solver's files
	int node_num = 0;
	if (rank % CORES_PER_NODE == 0) {
		cout << "process " << rank << " is making dir " << LOCAL_DIR << " on node " << node_num << endl;
		string system_str = "mkdir " + LOCAL_DIR;
		cout << "system_str : " << system_str << endl;
		exec(system_str);
		//system_str = "rm " + LOCAL_DIR + "id-*";
		//cout << "system_str : " << system_str << endl;
		//exec(system_str);
		node_num++;
		files_to_copy.push_back(solver_file_name);
		for (auto file_name : files_to_copy) {
			string system_str = "cp " + file_name + " " + LOCAL_DIR;
			cout << "system_str : " << system_str << endl;
			exec(system_str);
		}
	}
	
	// control or computing process
	if (rank == 0) {
		cout << "solver_file_name : " << solver_file_name << endl;
		cout << "cnf_file_name : "    << cnf_file_name << endl;
		cout << "cubes_file_name : "  << cubes_file_name << endl;
		cout << "cube_cpu_limit : "   << cube_cpu_lim_str << endl;
		controlProcess(corecount, cubes_file_name, cube_cpu_lim_str);
	}
	else
		computingProcess(rank, solver_file_name, cnf_file_name, cubes_file_name, cube_cpu_lim_str);
	
	return 0;
}

bool hasPrefix( string str, string prefix, string &value )
{
	int found = str.find( prefix );
	if ( found != -1 ) {
		value = str.substr( found + prefix.length( ) );
		return true;
	}
	return false;
}

string intToStr(const int x)
{
	stringstream sstream;
	sstream << x;
	return sstream.str();
}

vector<wu> readAndSortCubes(const string cubes_file_name)
{
	vector<wu> wu_vec = readCubes(cubes_file_name);
	sort(wu_vec.begin(), wu_vec.end(), compareByCubeSize);
	return wu_vec;
}

vector<wu> readCubes(const string cubes_file_name)
{
	vector<wu> res_wu_cubes;
	ifstream cubes_file(cubes_file_name);
	if (!cubes_file.is_open()) {
		cerr << "error: cubes_file " << cubes_file_name << " wasn't opened\n";
		exit(-1);
	}
	string str;
	stringstream sstream;
	vector<wu> wu_vec;
	int wu_id = 0;
	while (getline(cubes_file, str)) {
		sstream << str;
		string word;
		wu cur_wu;
		while (sstream >> word) {
			if ((word == "a") || (word == "0"))
				continue;
			int ival;
			istringstream(word) >> ival;
			cur_wu.cube.push_back(ival);
		}
		sstream.str(""); sstream.clear();
		cur_wu.status = NOT_STARTED;
		cur_wu.id = wu_id;
		cur_wu.processing_time = -1;
		cur_wu.result = -1;
		res_wu_cubes.push_back(cur_wu);
		wu_id++;
	}
	cubes_file.close();
	
	if (!res_wu_cubes.size()) {
		cerr << "wu_vec.size() == 0";
		MPI_Abort(MPI_COMM_WORLD, 0);
		exit(1);
	}

	return res_wu_cubes;
}

void controlProcess(const int corecount, const string cubes_file_name, const string cube_cpu_lim_str)
{
	double start_time = MPI_Wtime();
	vector<wu> wu_vec = readAndSortCubes(cubes_file_name);

	cout << "wu_vec size : " << wu_vec.size() << endl;
	cout << "first cubes : " << endl;
	for (unsigned i = 0; i < 3; i++) {
		for (auto x : wu_vec[i].cube)
			cout << x << " ";
		cout << endl;
	}
	
	// erase progress file
	string control_process_ofile_name = "!total_progress";
	ofstream control_process_ofile(control_process_ofile_name, ios_base::out);
	control_process_ofile.close();
	
	// start time in string
	time_t rawtime;
	struct tm * timeinfo;
	char time_char_arr[TIME_BUFFER_SIZE];
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(time_char_arr,sizeof(time_char_arr),"%d%m%Y%H%M%S",timeinfo);
	string time_str(time_char_arr);
	
	// send start time to all computing processes
	for (int i = 1; i < corecount; i++)
		MPI_Send(time_char_arr, TIME_BUFFER_SIZE, MPI_CHAR, i, 0, MPI_COMM_WORLD);

	// send a wu to every computing process
	int sending_id = 0;
	for (int i = 0; i < corecount - 1; i++) {
		sendWU(wu_vec, sending_id, i + 1);
		sending_id++;
	}
	
	// receive results and send back new WUs
	int wu_id, res;
	double time;
	double result_writing_time = -1;
	int stop_mes = -1;
	MPI_Status status, current_status;
	bool is_SAT = false;
	while (total_processed_wus < wu_vec.size()) {
		// receive result
		MPI_Recv(&wu_id, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		current_status = status;
		MPI_Recv(&res,  1, MPI_INT, current_status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&time, 1, MPI_DOUBLE, current_status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);

		// send back a new WU
		if (sending_id < wu_vec.size()) {
			sendWU(wu_vec, sending_id, current_status.MPI_SOURCE);
			sending_id++;
		}
		else {
			cout << "sending stop message to computing process " << current_status.MPI_SOURCE << endl;
			MPI_Send(&stop_mes, 1, MPI_INT, current_status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		}

		wu_vec[wu_id].status = PROCESSED;
		wu_vec[wu_id].result = res;
		wu_vec[wu_id].processing_time = time;
		total_processed_wus++;

		if (verb) {
			cout << "got result, time " << res << ", " << time << " for task id " << wu_id << endl;
			cout << "current_status.MPI_SOURCE : " << current_status.MPI_SOURCE << endl;
			cout << "total_processed_wus : " << total_processed_wus << endl;
		}
		
		if (res == SAT) {
				is_SAT = true;
				break;
		}
	
		// write results to a file not more often than every 100 seconds
		if ((result_writing_time < 0) || (MPI_Wtime() - result_writing_time > REPORT_EVERY_SEC)) {
			writeInfoOutFile(control_process_ofile_name, wu_vec, start_time, cube_cpu_lim_str);
			writeProcessingInfo(wu_vec);
			result_writing_time = MPI_Wtime();
		}
	}
	
	writeInfoOutFile(control_process_ofile_name, wu_vec, start_time, cube_cpu_lim_str);
	cout << "control process finished" << endl;

	writeProcessingInfo(wu_vec);
	
	// remove temporary files
	string system_str = "rm " + LOCAL_DIR + "id-" + time_str + "-*";
	cout << "system_str : " << system_str << endl;
	exec(system_str);
	
	if (is_SAT) {
		MPI_Abort(MPI_COMM_WORLD, 0);
	} else {
		string inter_cubes_file_name = "!interrupted_" + cubes_file_name;
		inter_cubes_file_name.erase(remove(inter_cubes_file_name.begin(), inter_cubes_file_name.end(), '.'), inter_cubes_file_name.end());
		inter_cubes_file_name.erase(remove(inter_cubes_file_name.begin(), inter_cubes_file_name.end(), '/'), inter_cubes_file_name.end());
		ofstream inter_cubes_file(inter_cubes_file_name);
		for (auto &cur_wu : wu_vec) {
			if (cur_wu.status != PROCESSED) {
				cerr << "cur_wu.status != PROCESSED" << endl;
				MPI_Abort(MPI_COMM_WORLD, 0);
			}
			if (cur_wu.result == INDET) {
				inter_cubes_file << "a ";
				for (auto lit : cur_wu.cube)
					inter_cubes_file << lit << " ";
				inter_cubes_file << "0" << endl;
			}
		}
		inter_cubes_file.close();
	}
	
	MPI_Finalize();
}

void sendWU(vector<wu> &wu_vec, const int wu_id, const int computing_process_id)
{
	if ( verb )
		cout << "sending task id " << wu_id << " to process " << computing_process_id << endl;

	MPI_Request request;
	//MPI_Send( &wu_id, 1, MPI_INT, computing_process_id, 0, MPI_COMM_WORLD );
	MPI_Isend( &wu_id, 1, MPI_INT, computing_process_id, 0, MPI_COMM_WORLD, &request );
	wu_vec[wu_id].status = IN_PROGRESS;
	/*int flag = 0;
	MPI_Status status;
	for (;;) {
		MPI_Test( &request, &flag, &status );
		if ( flag )
			break;
		//cout << "waiting for receiving of a sent message" << endl;
	}*/
	if ( verb )
		cout << "wu_vec[wu_id].status " << wu_vec[wu_id].status << endl;
}

void writeInfoOutFile(const string control_process_ofile_name, vector<wu> wu_vec, const double start_time, 
	const string cube_cpu_lim_str)
{
	double min_solving_time_unsat = 1e+308;
	double max_solving_time_unsat = -1;
	double avg_solving_time_unsat = -1;
	double sum_time_unsat = 0.0;
	double min_solving_time_inter_march = 1e+308;
	double max_solving_time_inter_march = -1;
	double avg_solving_time_inter_march = -1;
	double sum_time_inter_march = 0.0;
	int k = 0;
	int sat_cubes = 0;
	int unsat_cubes = 0;
	int indet_cubes = 0;
	int inter_march_cubes = 0;
	
	for (auto cur_wu : wu_vec) {
		if (cur_wu.status != PROCESSED)
			continue;
		k++;
		if (cur_wu.result == UNSAT) {
			unsat_cubes++;
			max_solving_time_unsat = cur_wu.processing_time > max_solving_time_unsat ? cur_wu.processing_time : max_solving_time_unsat;
			min_solving_time_unsat = cur_wu.processing_time < min_solving_time_unsat ? cur_wu.processing_time : min_solving_time_unsat;
			sum_time_unsat += cur_wu.processing_time;
		}
		else if (cur_wu.result == INDET) {
			indet_cubes++;
			double cube_cpu_lim;
			istringstream(cube_cpu_lim_str) >> cube_cpu_lim;
			if (cur_wu.processing_time < cube_cpu_lim - 10.0) {
				inter_march_cubes++;
				max_solving_time_inter_march = cur_wu.processing_time > max_solving_time_inter_march ? cur_wu.processing_time : max_solving_time_inter_march;
				min_solving_time_inter_march = cur_wu.processing_time < min_solving_time_inter_march ? cur_wu.processing_time : min_solving_time_inter_march;
				sum_time_inter_march += cur_wu.processing_time;
			}
		}
		else if (cur_wu.result == SAT) {
			sat_cubes++;
			string ofile_name = "!sat_cube_id_" + intToStr(cur_wu.id);
			ofstream ofile(ofile_name, ios_base::out);
			
			ofile << "SAT" << endl;
			ofile << "time : " << cur_wu.processing_time << " s" << endl;
			ofile << "cube id : " << cur_wu.id << endl;
			ofile << "cube : " << endl;
			for (auto &x : cur_wu.cube)
				ofile << x << " ";
			ofile << endl;
			ofile.close();
		}
	}
	if (k != total_processed_wus) {
		cerr << "k != total_processed_wus" << endl;
		cerr << k << " != " << total_processed_wus << endl;
		MPI_Abort(MPI_COMM_WORLD, 0);
		exit(-1);
	}
	if (sum_time_unsat > 0)
		avg_solving_time_unsat = sum_time_unsat / unsat_cubes;
	if (sum_time_inter_march > 0)
		avg_solving_time_inter_march = sum_time_inter_march / inter_march_cubes;
	
	double percent_val;
	ofstream control_process_ofile(control_process_ofile_name, ios_base::app);
	control_process_ofile << endl << "***" << endl;
	control_process_ofile << "elapsed time : " << MPI_Wtime() - start_time << endl;
	control_process_ofile << "total WUs : " << wu_vec.size() << endl;
	percent_val = double(total_processed_wus * 100) / (double)wu_vec.size();
	control_process_ofile << "total_processed_wus : " << total_processed_wus
		<< ", i.e. " << percent_val << " %" << endl;
	control_process_ofile << "sat_cubes : " << sat_cubes << endl;
	control_process_ofile << "indet_cubes : " << indet_cubes << endl;
	control_process_ofile << "unsat_cubes : " << unsat_cubes << endl;
	control_process_ofile << "min_solving_time_unsat : " << min_solving_time_unsat << endl;
	control_process_ofile << "max_solving_time_unsat : " << max_solving_time_unsat << endl;
	control_process_ofile << "avg_solving_time_unsat : " << avg_solving_time_unsat << endl;
	control_process_ofile << "inter_march_cubes (part of indet) : " << inter_march_cubes << endl;
	control_process_ofile << "min_solving_time_inter_march : " << min_solving_time_inter_march << endl;
	control_process_ofile << "max_solving_time_inter_march : " << max_solving_time_inter_march << endl;
	control_process_ofile << "avg_solving_time_inter_march : " << avg_solving_time_inter_march << endl;
	control_process_ofile << endl;
	control_process_ofile.close();
}

string exec(const string cmd_str)
{
	string result = "";
	char* cmd = new char[cmd_str.size() + 1];
	for (unsigned i = 0; i < cmd_str.size(); i++)
		cmd[i] = cmd_str[i];
	cmd[cmd_str.size()] = '\0';
	FILE* pipe = popen(cmd, "r");
	delete[] cmd;
	if (!pipe) return "ERROR";
	char buffer[128];
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
	return result;
}

int getResultFromFile(const string out_name)
{
	int result = INDET;
	ifstream out_file(out_name);
	if (out_file.is_open()) {
		string str;
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
	}
	else
		cerr << "out file " << out_name << " was not opened" << endl;
	/*if (time == -1) {
		cerr << "solving time == -1" << endl;
		cerr << endl;
		MPI_Abort(MPI_COMM_WORLD, 0);
		exit(-1);
	}*/
	return result;
}

void computingProcess(const int rank, const string solver_file_name, const string cnf_file_name, 
					  const string cubes_file_name, const string cube_cpu_lim_str)
{
	string rank_str = intToStr(rank);

	string log_file_name = "log_process_" + rank_str;
	ofstream log_file;

	if ( verb ) {
		log_file.open(log_file_name, ios_base::out);
		log_file.close();
		log_file.clear();
	}

	string local_solver_file_name = LOCAL_DIR;
	if (solver_file_name.find("./") != string::npos)
		local_solver_file_name += solver_file_name.substr(solver_file_name.find("./") + 2);
	else
		local_solver_file_name += solver_file_name;
	if (rank == 1)
		cout << "local_solver_file_name : " << local_solver_file_name << endl;

	vector<wu> wu_vec = readAndSortCubes(cubes_file_name);
	
	stringstream cnf_sstream;
	ifstream cnf_file(cnf_file_name);
	string str;
	unsigned cnf_main_clauses = 0;
	unsigned cnf_main_variables = 0;
	while (getline(cnf_file, str)) {
		if ((str.size() == 0) || (str[0] == 'p') || (str[0] == 'c'))
			continue;
		cnf_sstream << str << endl;
		cnf_main_clauses++;
		stringstream sstream;
		sstream << str;
		vector<int> vec;
		int ival;
		while (sstream >> ival) {
			int abs_ival = abs(ival);
			cnf_main_variables = (abs_ival > cnf_main_variables) ? abs_ival : cnf_main_variables;
		}
	}
	cnf_file.close();
	
	/*string base_path = exec("echo $PWD");
	base_path.erase(remove(base_path.begin(), base_path.end(), '\r'), base_path.end());
	base_path.erase(remove(base_path.begin(), base_path.end(), '\n'), base_path.end());
	solver_file_name = base_path + "/" + solver_file_name;
	cnf_file_name = base_path + "/" + cnf_file_name;*/

	MPI_Status status;
	
	char time_char_arr[TIME_BUFFER_SIZE];
	MPI_Recv( time_char_arr, TIME_BUFFER_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status );
	string time_str = time_char_arr;

	int wu_id = -1;
	for (;;) {
		if (verb) {
			log_file.open(log_file_name, ios_base::app);
			log_file << "waiting for a task" << endl;
			log_file.close();
			log_file.clear();
		}
		
		MPI_Recv( &wu_id, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status );

		if ( verb ) {
			log_file.open(log_file_name, ios_base::app);
			log_file << "got task id " << wu_id << endl;
			log_file.close();
			log_file.clear();
		}

		//cout << "received wu_id " << wu_id << endl;
		if (wu_id == -1) {// stop message
			cout << "computing prosess " << rank << " got the stop message" << endl;
			break;
		}
		
		string wu_id_str = intToStr(wu_id);
		string local_cnf_file_name = LOCAL_DIR + "id-" + time_str + '-' + wu_id_str + "-cnf";
		
		stringstream cube_sstream;
		for (auto x : wu_vec[wu_id].cube)
			cube_sstream << x << " 0" << endl;
		
		ofstream local_cnf_file(local_cnf_file_name, ios_base::out);
		local_cnf_file << "p cnf " << cnf_main_variables << " " << cnf_main_clauses + wu_vec[wu_id].cube.size() << endl;
		local_cnf_file << cnf_sstream.str();
		local_cnf_file << cube_sstream.str();
		local_cnf_file.close();
		
		string system_str;
		if (local_solver_file_name.find(".sh") != string::npos) {
			// cube_cpu_lim_str is used as cpu-lim for an incremental SAT solver
			system_str = local_solver_file_name + " " + local_cnf_file_name
				+ " " + wu_id_str + " " + cube_cpu_lim_str;
		}
		else {
			system_str = LOCAL_DIR + "timelimit -t " + cube_cpu_lim_str + " -T 1 " 
				+ local_solver_file_name + " " + local_cnf_file_name;
		}
		string local_out_file_name = LOCAL_DIR + "id-" + time_str + '-' + wu_id_str + "-out";
		fstream local_out_file;
		local_out_file.open(local_out_file_name, ios_base::out);

		if ( verb ) {
			log_file.open(log_file_name, ios_base::app);
			log_file << "system_str : " << system_str << endl;
			log_file.close();
			log_file.clear();
		}

		double elapsed_solving_time = MPI_Wtime();
		stringstream sstream;
		sstream << exec(system_str);
		elapsed_solving_time = MPI_Wtime() - elapsed_solving_time;

		if ( verb ) {
			log_file.open(log_file_name, ios_base::app);
			log_file << "out : " << sstream.str() << endl;
			log_file << "elapsed_solving_time : " << elapsed_solving_time << endl;
			log_file.close();
			log_file.clear();
		}

		local_out_file << sstream.str();
		local_out_file.close();
		local_out_file.clear();
		int res = INDET;
		double cube_cpu_lim = -1.0;
		istringstream(cube_cpu_lim_str) >> cube_cpu_lim;
		res = getResultFromFile(local_out_file_name);
		// remove the temporary cnf file
		if (res == SAT) {
			system_str = "cp " + local_out_file_name + " ./!sat_out_id_" + wu_id_str;
			exec(system_str);
			system_str = "cp " + LOCAL_DIR + "id-" + time_str + '-' + wu_id_str + "-*" + " ./";
			exec(system_str);
		}
		else if (elapsed_solving_time > cube_cpu_lim + 10.0) {
			cout << "extra elapsed_solving_time : " << elapsed_solving_time << endl;
			system_str = "cp " + local_out_file_name + " ./!extra_time_out_id_" + wu_id_str;
			exec(system_str);
		}
		else {
			system_str = "rm " + LOCAL_DIR + "id-" + time_str + '-' + wu_id_str + "-*";
			exec(system_str);
		}

		if ( verb ) {
			log_file.open(log_file_name, ios_base::app);
			log_file << "before sending result " << res << " for task id " << wu_id << endl;
			log_file.close();
			log_file.clear();
		}
		
		// send calculated result to the control process
		//cout << "sending wu_id " << wu_id << endl;
		//cout << "sending res " << res << endl;
		MPI_Send( &wu_id,                1, MPI_INT,    0, 0, MPI_COMM_WORLD);
		MPI_Send( &res,                  1, MPI_INT,    0, 0, MPI_COMM_WORLD);
		MPI_Send( &elapsed_solving_time, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

		if ( verb ) {
			log_file.open(log_file_name, ios_base::app);
			log_file << "after sending result " << res << " for task id " << wu_id << endl;
			log_file.close();
			log_file.clear();
		}
	}
	MPI_Finalize();
}

void writeProcessingInfo(vector<wu> &wu_vec)
{
	ofstream ofile("!processing_info");
	ofile << "cube_id cube_status cube_result cube_time" << endl;
	for (auto &cur_wu : wu_vec)
		ofile << cur_wu.id << " " << cur_wu.status << " " << cur_wu.result << " " << cur_wu.processing_time << endl;
	ofile.close();
	ofile.clear();
}