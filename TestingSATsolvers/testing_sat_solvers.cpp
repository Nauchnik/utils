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
	std::string name;
	double avg_time;
	double min_time;
	double max_time;
};

std::string get_cpu_lim_str( std::string solvers_dir, std::string solver_name, 
					    std::string maxtime_seconds_str, std::string nof_threads_str )
{
	std::string result_str;
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
		result_str = "-cpu-lim=" +  maxtime_seconds_str;
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

	double clock_solving_time;
	unsigned int nthreads = std::thread::hardware_concurrency();
	std::cout << "nthreads " << nthreads << std::endl;
	std::stringstream sstream;
	sstream << nthreads;
	std::string nof_threads_str = sstream.str();
	sstream.clear(); sstream.str("");
	std::cout << "nof_threads_str " << nof_threads_str << std::endl;
	
	std::string system_str, current_out_name, str;
	unsigned copy_from, copy_to;
	std::fstream current_out;
	double cur_time, avg_time = 0;
	std::string maxtime_seconds_str;

	if ( argc < 3 ) {
		std::cout << "Usage: [solvers_path] [cnfs_path] [maxtime_seconds_one_problem]" << std::endl;
		return 1;
	}

	if ( argc == 3 ) {
		maxtime_seconds_str = "600"; 
		std::cout << "maxtime_seconds was set to default == 600 seconds" << std::endl;
	} else
		maxtime_seconds_str = argv[3];

	std::string solvers_dir, cnfs_dir;
	solvers_dir = argv[1];
	cnfs_dir = argv[2];
	std::cout << "solvers_dir "     << solvers_dir         << std::endl;
	std::cout << "cnfs_dir "        << cnfs_dir            << std::endl;
	std::cout << "maxtime_seconds " << maxtime_seconds_str << std::endl;
	
	std::vector<std::string> solver_files_names = std::vector<std::string>();
	std::vector<std::string> cnf_files_names = std::vector<std::string>();
	
	if ( !Addit_func::getdir( solvers_dir, solver_files_names ) ) { return 1; }
	if ( !Addit_func::getdir( cnfs_dir, cnf_files_names ) ) { return 1; };
	sort( solver_files_names.begin(), solver_files_names.end() );
	sort( cnf_files_names.begin(), cnf_files_names.end() );
	
	std::cout << std::endl << "solver_files_names :" << std::endl;
	for ( std::vector<std::string> :: iterator it = solver_files_names.begin(); it != solver_files_names.end(); it++ )
		std::cout << *it << std::endl;
	std::cout << std::endl << "cnf_files_names :" << std::endl;
	for ( std::vector<std::string> :: iterator it = cnf_files_names.begin(); it != cnf_files_names.end(); it++ )
		std::cout << *it << std::endl;
	
	std::vector<solver_info> solver_info_vec;
	solver_info cur_solver_info;
	std::vector<unsigned> sat_count_vec;
	sat_count_vec.resize( solver_files_names.size() );
	for ( std::vector<unsigned> :: iterator it = sat_count_vec.begin(); it != sat_count_vec.end(); it++ )
		*it = 0;
	
	bool isTimeStr, isSAT;
	unsigned solved_problems_count = 0;
	double sum_time, min_time, max_time;
	std::vector< std::vector<std::string> > solver_cnf_times_str;
	std::string solver_time_str;
	std::stringstream convert_sstream;
	solver_cnf_times_str.resize( solver_files_names.size() );
	for ( unsigned i=0; i < solver_files_names.size(); i++ ) {
		sum_time = 0;
		solved_problems_count = 0;
		for ( unsigned j=0; j < cnf_files_names.size(); j++ ) {
			current_out_name = "out_" + solver_files_names[i] + "_" + cnf_files_names[j];
			system_str = get_cpu_lim_str( solvers_dir, solver_files_names[i], maxtime_seconds_str, nof_threads_str ) + 
				         " ./" + cnfs_dir + "/" + cnf_files_names[j] + get_params_str( solver_files_names[i] );
			std::cout << system_str << std::endl;
					    // + " &> ./" + current_out_name;
			//std::cout << system_str << std::endl;
			//cout << "system_result_stream" << endl;
			//cout << system_result_stream.str() << endl;
			//system( system_str.c_str( ) );
			current_out.open( current_out_name.c_str(), std::ios_base :: out );
			std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
			current_out << Addit_func::exec( system_str );
			std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			clock_solving_time = time_span.count();
			current_out.close();
			std::cout << "clock_solving_time " << clock_solving_time << std::endl;
			solver_time_str = cnf_files_names[j];
			convert_sstream << " " << clock_solving_time;
			solver_time_str += convert_sstream.str();
			convert_sstream.str(""); convert_sstream.clear();
			solver_cnf_times_str[i].push_back( solver_time_str );
			
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
					std::cout << solver_files_names[i] << " : " << sat_count_vec[i] << " sat from " << 
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
					std::cout << "time str " << str << std::endl;
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

		std::string solver_out_file_name = "out_" + solver_files_names[i] + "_total";
		std::ofstream solver_out_file( solver_out_file_name.c_str() );
		for ( unsigned t = 0; t < solver_cnf_times_str[i].size(); t++ )
			solver_out_file << solver_cnf_times_str[i][t] << std::endl;
		solver_out_file.close(); solver_out_file.clear();
		
		cur_solver_info.name = solver_files_names[i];
		cur_solver_info.avg_time = avg_time;
		cur_solver_info.min_time = min_time;
		cur_solver_info.max_time = max_time;
		solver_info_vec.push_back( cur_solver_info );
	}

	std::cout << "*** Final statistics ***" << std::endl;
	std::cout << "Total problems " << cnf_files_names.size() << std::endl;
	for ( std::vector<solver_info> :: iterator it = solver_info_vec.begin(); it != solver_info_vec.end(); it++ ) {
		std::cout << (*it).name << std::endl;
		std::cout << "  avg_time " << (*it).avg_time << " s" << std::endl;
		std::cout << "  min_time " << (*it).min_time << " s" << std::endl;
		std::cout << "  max_time " << (*it).max_time << " s" << std::endl;
	}
	return 0;
}