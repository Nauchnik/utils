// Oleg Zaikin, 1.04.2020 (Irkutsk)
/* Copyright 2020, 2021 Oleg Zaikin */

// Solving cubes in parallel via OpenMP.

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <omp.h>

std::string version = "0.1.0";

enum status{ NOT_STARTED = -1, IN_PROGRESS = 0, PROCESSED = 1};
enum result{ UNSAT = 2, SAT = 3, INDET = 4 };

#define cube_t std::vector<int> 

struct workunit {
	int id;
  status stts;
	result rslt;
	cube_t cube;
	double time;
	workunit() : id(-1), stts(NOT_STARTED), rslt(INDET), cube(), time(-1) {};
	void print() {
		for (auto &c : cube) std::cout << c << " ";
		std::cout << std::endl;
	}
};

bool compareByCubeSize(const workunit &a, const workunit &b) {
	return a.cube.size() > b.cube.size();
}

std::vector<workunit> readCubes(const std::string cubes_file_name);
/*void computingProcess(const int rank, const string solver_file_name, const string cnf_file_name, 
					  const string cubes_file_name, const string cube_cpu_lim_str);
void writeInfoOutFile(const string process_ofile_name, vector<wu> wu_vec, 
	const double start_time, const string cube_cpu_lim_str);
int read_solver_result(const string out_name);
void writeProcessingInfo(vector<wu> &wu_vec);
string exec(const string cmd_str);
/*/

void print_usage() {
	std::cout << "Usage : parallel_cubes solver-name cnf-name cubes-name " <<
		           "cube-time-limit [--verb]" << std::endl;
}

void print_version() {
	std::cout << "version: " << version << std::endl;
}

int main(const int argc, const char *argv[]) {

	std::vector<std::string> str_argv;

	for (int i=0; i < argc; ++i) str_argv.push_back(argv[i]);
	assert(str_argv.size() == argc);
	if (argc == 2 and str_argv[1] == "-h") {
		print_usage();
		std::exit(EXIT_SUCCESS);
	}
	if (argc == 2 and str_argv[1] == "-v") {
		print_version();
		std::exit(EXIT_SUCCESS);
	}
	if (argc < 5) {
		print_usage();
		std::exit(EXIT_FAILURE);
	}

	std::string solver_name       = str_argv[1];
	std::string cnf_name	        = str_argv[2];
	std::string cubes_name	      = str_argv[3];
	const double cube_time_lim = std::stod(str_argv[4]);
	assert(cube_time_lim > 0);
	bool verb = (argc == 6 and str_argv[5] == "--verb") ? true : false;
	std::cout << "solver_name : "   << solver_name   << std::endl;
	std::cout << "cnf_name : "      << cnf_name      << std::endl;
	std::cout << "cubes_name : "    << cubes_name    << std::endl;
	std::cout << "cube_time_lim : " << cube_time_lim << std::endl;
	std::cout << "verbosity : "     << verb          << std::endl;

	int threads = omp_get_num_threads();
	std::cout << "threads : " << threads << std::endl << std::endl;

	auto start = std::chrono::steady_clock::now();

	std::vector<workunit> wu_vec = readCubes(cubes_name);
	// Sort cubes by size in descending order:
	std::stable_sort(wu_vec.begin(), wu_vec.end(), compareByCubeSize);
	std::cout << "cubes : " << wu_vec.size() << std::endl;
	std::cout << "first cubes : " << std::endl;
	for (unsigned i = 0; i < 3; i++) wu_vec[i].print();
	
	// erase progress file
	std::string process_ofile_name = "!total_progress";
	std::ofstream process_ofile(process_ofile_name, std::ios_base::out);
	process_ofile.close();
	
	// total_processed_wus

	/*wu_vec[wu_id].status = PROCESSED;
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
		writeInfoOutFile(process_ofile_name, wu_vec, start_time, cube_cpu_lim_str);
		writeProcessingInfo(wu_vec);
		result_writing_time = MPI_Wtime();
	}
	
	writeInfoOutFile(process_ofile_name, wu_vec, start_time, cube_cpu_lim_str);
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
	}*/

	auto end = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

	std::cout << "elapsed : " << elapsed << " seconds" << std::endl;

	return 0;
}

// Read cubes from a given file
std::vector<workunit> readCubes(const std::string cubes_name) {
	std::vector<workunit> wu_cubes;
	std::ifstream cubes_file(cubes_name);
	if (!cubes_file.is_open()) {
		std::cerr << "cubes_file " << cubes_name << " wasn't opened\n";
		std::exit(EXIT_FAILURE);
	}
	std::string str;
	std::stringstream sstream;
	std::vector<workunit> wu_vec;
	int id = 0;
	while (getline(cubes_file, str)) {
		sstream << str;
		std::string word;
		workunit wu;
		assert(wu.id == -1);
		assert(wu.stts == NOT_STARTED);
		assert(wu.rslt == INDET);
		assert(wu.time == -1);
		while (sstream >> word) {
			if (word == "a" or word == "0") continue;
			wu.cube.push_back(std::stoi(word));
		}
		sstream.str(""); sstream.clear();
		wu.id = id++;
		wu_vec.push_back(wu);
	}
	cubes_file.close();
	if (!wu_vec.size()) {
		std::cerr << "wu_vec.size() == 0";
		std::exit(EXIT_FAILURE);
	}
	return wu_vec;
}

/*
void write_info(const string process_ofile_name, vector<wu> wu_vec, const double start_time, 
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
	ofstream process_ofile(process_ofile_name, ios_base::app);
	process_ofile << endl << "***" << endl;
	process_ofile << "elapsed time : " << MPI_Wtime() - start_time << endl;
	process_ofile << "total WUs : " << wu_vec.size() << endl;
	percent_val = double(total_processed_wus * 100) / (double)wu_vec.size();
	process_ofile << "total_processed_wus : " << total_processed_wus
		<< ", i.e. " << percent_val << " %" << endl;
	process_ofile << "sat_cubes : " << sat_cubes << endl;
	process_ofile << "indet_cubes : " << indet_cubes << endl;
	process_ofile << "unsat_cubes : " << unsat_cubes << endl;
	process_ofile << "min_solving_time_unsat : " << min_solving_time_unsat << endl;
	process_ofile << "max_solving_time_unsat : " << max_solving_time_unsat << endl;
	process_ofile << "avg_solving_time_unsat : " << avg_solving_time_unsat << endl;
	process_ofile << "inter_march_cubes (part of indet) : " << inter_march_cubes << endl;
	process_ofile << "min_solving_time_inter_march : " << min_solving_time_inter_march << endl;
	process_ofile << "max_solving_time_inter_march : " << max_solving_time_inter_march << endl;
	process_ofile << "avg_solving_time_inter_march : " << avg_solving_time_inter_march << endl;
	process_ofile << endl;
	process_ofile.close();
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

int read_solver_result(const std::string out_name)
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
*/

void write_processing_info(const std::vector<workunit> &wu_vec)
{
	std::ofstream ofile("!cubes_processing_info");
	ofile << "id status result time" << std::endl;
	for (auto &wu : wu_vec)
		ofile << wu.id << " " << wu.stts << " " << wu.rslt << " " <<
				     wu.time << std::endl;
	ofile.close();
}
