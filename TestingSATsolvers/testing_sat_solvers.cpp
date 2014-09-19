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

using namespace Addit_func;

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

string get_cpu_lim_str( std::string solvers_dir, std::string solver_name, 
					    std::string maxtime_seconds_str, std::string nof_threads_str )
{
	string result_str;
	if ( ( solver_name.find( "minisat//minisat" ) != std::string::npos ) || 
	     ( solver_name.find( "minisat_simp//minisat_simp" ) != std::string::npos ) )
	{
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

std::string get_params_str( std::string solver_name )
{
	std::string result_str;
	if ( solver_name.find( "CSCC" ) != std::string::npos ) {
		result_str = " 1";
	}
	return result_str;
}

int main( int argc, char **argv )
{
	
#ifdef _DEBUG
	argc = 3;
	argv[1] = "solvers";
	argv[2] = "cnfs";
#endif
	using namespace std::chrono;

	double clock_solving_time;
	unsigned int nthreads = std::thread::hardware_concurrency();
	std::cout << "nthreads " << nthreads << std::endl;
	stringstream sstream;
	sstream << nthreads;
	std::string nof_threads_str = sstream.str();
	sstream.clear(); sstream.str("");
	std::cout << "nof_threads_str " << nof_threads_str << std::endl;
	
	string system_str, current_out_name, current_res_name, str;
	unsigned copy_from, copy_to;
	std::fstream current_out;
	double cur_time, avg_time = 0;
	string maxtime_seconds_str;

	if ( argc < 3 ) {
		std::cout << "Usage: [solvers_path] [cnfs_path] [maxtime_seconds_one_problem]" << std::endl;
		return 1;
	}

	if ( argc == 3 ) {
		maxtime_seconds_str = "600"; 
		std::cout << "maxtime_seconds was set to default == 600 seconds" << std::endl;
	} else
		maxtime_seconds_str = argv[3];

	string solvers_dir, cnfs_dir;
	solvers_dir = argv[1];
	cnfs_dir = argv[2];
	std::cout << "solvers_dir "     << solvers_dir         << std::endl;
	std::cout << "cnfs_dir "        << cnfs_dir            << std::endl;
	std::cout << "maxtime_seconds " << maxtime_seconds_str << std::endl;
	
	vector<string> solver_files_names = vector<string>();
	vector<string> cnf_files_names = vector<string>();
	
	if ( !Addit_func::getdir( solvers_dir, solver_files_names ) ) { return 1; }
	if ( !Addit_func::getdir( cnfs_dir, cnf_files_names ) ) { return 1; };
	sort( solver_files_names.begin(), solver_files_names.end() );
	sort( cnf_files_names.begin(), cnf_files_names.end() );
	
	std::cout << std::endl << "solver_files_names :" << std::endl;
	for ( auto &x : solver_files_names )
		std::cout << x << std::endl;
	std::cout << std::endl << "cnf_files_names :" << std::endl;
	for ( auto &x : cnf_files_names )
		std::cout << x << std::endl;
	
	vector<solver_info> solver_info_vec;
	solver_info cur_solver_info;
	std::vector<unsigned> sat_count_vec;
	sat_count_vec.resize( cnf_files_names.size() );
	for ( auto &x : sat_count_vec )
		x = 0;
	
	bool isTimeStr, isSAT;
	unsigned solved_problems_count = 0;
	double sum_time, min_time, max_time;
	for ( unsigned i=0; i < solver_files_names.size(); i++ ) {
		sum_time = 0;
		solved_problems_count = 0;
		for ( unsigned j=0; j < cnf_files_names.size(); j++ ) {
			current_out_name = "out_" + solver_files_names[i] + "_" + cnf_files_names[j];
			current_res_name = "res_" + solver_files_names[i] + "_" + cnf_files_names[j];
			system_str = get_cpu_lim_str( solvers_dir, solver_files_names[i], maxtime_seconds_str, nof_threads_str ) + 
				         " ./" + cnfs_dir + "/" + cnf_files_names[j] + get_params_str( solver_files_names[i] );
			std::cout << system_str << std::endl;
						 //+ " " + current_res_name 
					    // + " &> ./" + current_out_name;
			//std::cout << system_str << std::endl;
			//cout << "system_result_stream" << endl;
			//cout << system_result_stream.str() << endl;
			//system( system_str.c_str( ) );
			current_out.open( current_out_name.c_str(), std::ios_base :: out );
			high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
			current_out << exec( system_str );
			std::chrono::high_resolution_clock::time_point t2 = high_resolution_clock::now();
			std::chrono::duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
			clock_solving_time = time_span.count();
			current_out.close();
			std::cout << "clock_solving_time " << clock_solving_time << std::endl;
			
			current_out.open( current_out_name.c_str(), std::ios_base :: in );
			
			isSAT = false;
			cur_time = 0.0;
			while ( getline( current_out, str ) ) {
				if ( str.find("SATISFIABLE") != std::string::npos ) {
					isSAT = true;
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
					sstream.str(""); sstream.clear();
				}
			}
			current_out.close();
			
			if ( cur_time <= 0.0 ) 
				cur_time = clock_solving_time;
			std::cout << "cur_time " << cur_time << std::endl;
			sum_time += cur_time;
			if ( j == 0 ) {
				min_time = cur_time;
				max_time = cur_time;
			} else {
				min_time = ( cur_time < min_time ) ? cur_time : min_time;
				max_time = ( cur_time > max_time ) ? cur_time : max_time;
			}
			solved_problems_count++;
			std::cout << "solved_problems_count " << solved_problems_count << std::endl;
		}
		if ( isSAT ) {
			avg_time = sum_time / (double)solved_problems_count;
			std::cout << "cur_avg_time " << avg_time << std::endl;
			std::cout << "cur_min_time " << min_time << std::endl;
			std::cout << "cur_max_time " << max_time << std::endl;
		}
		else
			min_time = max_time = 0.0;
		
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