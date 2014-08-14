#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <thread>

#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif

using namespace std;

struct solver_info
{
	string name;
	double avg_time;
	double min_time;
	double max_time;
};

// ececute command via system process
std::string exec( std::string cmd_str ) {
	char* cmd = new char[cmd_str.size() + 1];
	strcpy( cmd, cmd_str.c_str() );
	cmd[cmd_str.size()] = '\0';
#ifdef _WIN32
    FILE* pipe = _popen(cmd, "r");
#else
	FILE* pipe = popen(cmd, "r");
#endif
	delete[] cmd;
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
#ifdef _WIN32
    _pclose(pipe);
#else
	pclose(pipe);
#endif
    return result;
}

int getdir( std::string dir, vector<std::string> &files )
{
    DIR *dp;
	string cur_name;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cout << endl << "Error in opening " << dir;
        return 1;
    }
    while ((dirp = readdir(dp)) != NULL) { 
		cur_name = string(dirp->d_name);
		if ( cur_name[0] != '.' ) files.push_back(cur_name); 
	}
    closedir(dp);
    return 0;
}

string get_cpu_lim_str( std::string solvers_dir, std::string solver_name, 
					    std::string maxtime_seconds_str, std::string nof_threads_str )
{
	string result_str;
	if ( solver_name.find( "minisat_simp" ) != std::string::npos ) {
		//std::cout << "minisat_simp detected" << std::endl;
		result_str =  "-cpu-lim=";
	}
	// glucose can't stop in time
	/*if ( solver_name.find( "glucose" ) != std::string::npos ) {
		std::cout << "glucose detected" << std::endl;
		result_str = "-cpu-lim=";
	}*/
	else if ( solver_name.find( "sinn" ) != std::string::npos ) {
		//std::cout << "sinn detected" << std::endl;
		return "-cpu-lim=" +  maxtime_seconds_str;
	}
	else if ( solver_name.find( "minisat_bit" ) != std::string::npos ) {
		//std::cout << "minisat_bit detected" << std::endl;
		result_str = "-cpu-lim=";
	}
	else if ( solver_name.find( "zenn" ) != std::string::npos ) {
		//std::cout << "zenn detected" << std::endl;
		result_str = "-cpu-lim=";
	}
	else if ( solver_name.find( "glueminisat" ) != std::string::npos ) {
		//std::cout << "glueminisat detected" << std::endl;
		result_str = "-cpu-lim=";
	}
	else if ( solver_name.find( "minigolf" ) != std::string::npos ) {
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
	else if ( ( solver_name.find( "lingeling" ) != std::string::npos ) && 
		      ( solver_name.find( "plingeling" ) == std::string::npos ) ) {
		//std::cout << "lingeling detected" << std::endl;
		result_str = "-t ";
	}

	
	if ( result_str == "" ) {
		std::cout << "unknown solver detected. using timelimit" << std::endl;
		result_str = "./timelimit -t " + maxtime_seconds_str + " -T 1 " + "./" + solvers_dir + "/" + solver_name;
	}
	else
		result_str = "./" + solvers_dir + "/" + solver_name + " " + result_str + maxtime_seconds_str;

	if ( solver_name.find( "dimetheus" ) != std::string::npos )
		result_str += " -formula";
	
	return result_str;
}

int main( int argc, char **argv )
{
	// debug
	/*argc = 3;
	argv[1] = "solvers";
	argv[2] = "cnfs";*/

	unsigned int nthreads = std::thread::hardware_concurrency();
	std::cout << "nthreads " << nthreads << std::endl;
	stringstream sstream;
	sstream << nthreads;
	std::string nof_threads_str = sstream.str();
	sstream.clear(); sstream.str("");
	std::cout << "nof_threads_str " << nof_threads_str << std::endl;
	
	string system_str, current_out_name, current_res_name, str;
	unsigned copy_from, copy_to;
	fstream current_out;
	double cur_time, avg_time = 0;
	string maxtime_seconds_str;

	vector<string> solver_files_names = vector<string>();
	vector<string> cnf_files_names = vector<string>();
	string solvers_dir, cnfs_dir;

	if ( argc < 3 ) {
		std::cout << "Usage: [solvers_path] [cnfs_path] [maxtime_seconds_one_problem]" << endl;
		return 1;
	}

	if ( argc == 3 ) {
		maxtime_seconds_str = "600"; 
		std::cout << "maxtime_seconds was set to default == 600 seconds" << endl;
	}

	solvers_dir = argv[1];
	cnfs_dir = argv[2];
	maxtime_seconds_str = argv[3];
	std::cout << "solvers_dir "     << solvers_dir         << endl;
	std::cout << "cnfs_dir "        << cnfs_dir            << endl;
	std::cout << "maxtime_seconds " << maxtime_seconds_str << endl;

	getdir( solvers_dir, solver_files_names );
	getdir( cnfs_dir, cnf_files_names );
	
	vector<solver_info> solver_info_vec;
	solver_info cur_solver_info;
	std::vector<unsigned> sat_count_vec;
	sat_count_vec.resize( solver_files_names.size() );
	for ( auto &x : sat_count_vec )
		x = 0;
	
	bool isTimeStr;
	unsigned solved_problems_count = 0;
	double sum_time, min_time, max_time;
	for ( unsigned i=0; i < solver_files_names.size(); i++ ) {
		sum_time = 0;
		solved_problems_count = 0;
		for ( unsigned j=0; j < cnf_files_names.size(); j++ ) {
			current_out_name = "out_" + solver_files_names[i] + "_" + cnf_files_names[j];
			current_res_name = "res_" + solver_files_names[i] + "_" + cnf_files_names[j];
			
			system_str = get_cpu_lim_str( solvers_dir, solver_files_names[i], maxtime_seconds_str, nof_threads_str ) + 
				         " ./" + cnfs_dir + "/" + cnf_files_names[j];
			std::cout << system_str << std::endl;
						 //+ " " + current_res_name 
					    // + " &> ./" + current_out_name;
			//std::cout << system_str << std::endl;
			//cout << "system_result_stream" << endl;
			//cout << system_result_stream.str() << endl;
			//system( system_str.c_str( ) );
			current_out.open( current_out_name.c_str(), ios_base :: out );
			current_out << exec( system_str );
			current_out.close();
			current_out.open( current_out_name.c_str(), ios_base :: in );
			
			while ( getline( current_out, str ) ) {
				if ( str.find("SATISFIABLE") != std::string::npos ) {
					sat_count_vec[i]++;
					std::cout << "SAT found" << std::endl;
					std::cout << "current_out_name " << current_out_name << std::endl;
					std::cout << "sat_count_vec" << std::endl;
					for ( unsigned t=0; t < sat_count_vec.size(); t++ )
						std::cout << solver_files_names[t] << " : " << sat_count_vec[t] << " sat from " << 
						             cnf_files_names.size() << std::endl;
				}
				isTimeStr = true;
				if ( str.find("CPU time") != std::string::npos ) {
					copy_from = str.find(":") + 2;
					copy_to = str.find(" s") - 1;
				}
				else if ( ( str.find("seconds") != std::string::npos ) && // lingeling format
						  ( str.find("MB") != std::string::npos ) && 
						  ( str.size() < 30 ) ) {
					copy_from = str.find("c ") + 2;
					copy_to = str.find(" seconds") - 1;
				}
				else if ( str.find("c Running time=") != std::string::npos ) { // glueSplit_clasp format
					copy_from = str.find("c Running time=") + 15;
					copy_to = str.size() - 1;
				}
				else
					isTimeStr = false;
				if ( isTimeStr ) {
					str = str.substr( copy_from, (copy_to-copy_from+1) );
					sstream << str;
					sstream >> cur_time;
					if ( cur_time == 0.0 ) {
						cur_time = 0.01; // if unit propagation
						std::cout << "Warning. cur_time == 0. changed to 0.1" << endl;
					}
					std::cout << "time " << cur_time << endl;
					sum_time += cur_time;
					if ( j == 0 ) {
						min_time = cur_time;
						max_time = cur_time;
					} else {
						min_time = ( cur_time < min_time ) ? cur_time : min_time;
						max_time = ( cur_time > max_time ) ? cur_time : max_time;
					}
					solved_problems_count++;
					std::cout << "solved_problems_count " << solved_problems_count << endl;
					avg_time = sum_time / (double)solved_problems_count;
					std::cout << "cur_avg_time " << avg_time << endl;
					std::cout << "cur_min_time " << min_time << endl;
					std::cout << "cur_max_time " << max_time << endl;
					sstream.str(""); sstream.clear();
				}
			}
			current_out.close();
		}
		cur_solver_info.name = solver_files_names[i];
		cur_solver_info.avg_time = avg_time;
		cur_solver_info.min_time = min_time;
		cur_solver_info.max_time = max_time;
		solver_info_vec.push_back( cur_solver_info );
	}

	std::cout << "*** Final statistics ***" << std::endl;
	std::cout << "Total problems " << cnf_files_names.size() << std::endl;
	for ( vector<solver_info> :: iterator it = solver_info_vec.begin(); it != solver_info_vec.end(); it++ ) {
		std::cout << (*it).name << std::endl;
		std::cout << "  avg_time " << (*it).avg_time << " s" << std::endl;
		std::cout << "  min_time " << (*it).min_time << " s" << std::endl;
		std::cout << "  max_time " << (*it).max_time << " s" << std::endl;
	}
	return 0;
}