#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

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

int getdir( string dir, vector<string> &files )
{
    DIR *dp;
	string cur_name;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << endl << "Error in opening " << dir;
        return 1;
    }
    while ((dirp = readdir(dp)) != NULL) { 
		cur_name = string(dirp->d_name);
		if ( cur_name[0] != '.' ) files.push_back(cur_name); 
	}
    closedir(dp);
    return 0;
}

int main( int argc, char **argv )
{
	// debug
	/*argc = 3;
	argv[1] = "solvers";
	argv[2] = "cnfs";*/

	string system_str, current_out_name, current_stream_name, str;
	unsigned copy_from, copy_to;
	ifstream current_out;
	double cur_time, avg_time = 0;
	stringstream sstream;
	string maxtime_seconds_str;

	vector<string> solver_files_names = vector<string>( );
	vector<string> cnf_files_names = vector<string>( );
	string solvers_dir, cnfs_dir;

	if ( argc < 3 ) {
		cout << "Usage: [solvers_path] [cnfs_path] [maxtime_seconds_one_problem]" << endl;
		return 1;
	}

	if ( argc == 3 ) {
		maxtime_seconds_str = "600"; 
		cout << "maxtime_seconds was set to default == 600 seconds" << endl;
	}

	solvers_dir = argv[1];
	cnfs_dir = argv[2];
	maxtime_seconds_str = argv[3];
	cout << "solvers_dir "     << solvers_dir         << endl;
	cout << "cnfs_dir "        << cnfs_dir            << endl;
	cout << "maxtime_seconds " << maxtime_seconds_str << endl;

	getdir( solvers_dir, solver_files_names );
	getdir( cnfs_dir, cnf_files_names );

	vector<solver_info> solver_info_vec;
	solver_info cur_solver_info;
	unsigned solved_problems_count = 0;
	double sum_time, min_time, max_time;
	for ( unsigned i=0; i < solver_files_names.size(); i++ ) {
		sum_time = 0;
		solved_problems_count = 0;
		for ( unsigned j=0; j < cnf_files_names.size(); j++ ) {
			current_out_name = "out_" + solver_files_names[i] + "_" + cnf_files_names[j];
			current_stream_name = "stream_" + solver_files_names[i] + "_" + cnf_files_names[j];
			system_str = "./timelimit -t " + maxtime_seconds_str + " -T 1 " "./" + solvers_dir + "/" + 
				         solver_files_names[i] + " ./" + cnfs_dir + "/" + cnf_files_names[j] + 
						 " " + current_out_name + " > " + current_stream_name;
			cout << system_str << endl;
			system( system_str.c_str( ) );
			current_out.open( current_out_name.c_str() );
			while ( getline( current_out, str ) ) {
				if ( str.find("CPU time") != string::npos ) {
					copy_from = str.find(":") + 2;
					copy_to = str.find(" s") - 1;
					str = str.substr( copy_from, (copy_to-copy_from+1) );
					//cout << "time str " << str << endl;
					sstream << str;
					sstream >> cur_time;
					if ( cur_time == 0 ) {
						cur_time = 0.01; // if unit propagation
						cout << "Warning. cur_time == 0. changed to 0.1" << endl;
					}
					cout << "time " << cur_time << endl;
					sum_time += cur_time;
					if ( j == 0 ) {
						min_time = cur_time;
						max_time = cur_time;
					} else {
						min_time = ( cur_time < min_time ) ? cur_time : min_time;
						max_time = ( cur_time > max_time ) ? cur_time : max_time;
					}
					solved_problems_count++;
					cout << "solved_problems_count " << solved_problems_count << endl;
					avg_time = sum_time / (double)solved_problems_count;
					cout << "cur_avg_time " << avg_time << endl;
					cout << "cur_min_time " << min_time << endl;
					cout << "cur_max_time " << max_time << endl;
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

	cout << "*** Final statistics ***" << endl;
	cout << "Total problems " << cnf_files_names.size() << endl;
	for ( vector<solver_info> :: iterator it = solver_info_vec.begin(); it != solver_info_vec.end(); it++ ) {
		cout << (*it).name << endl;
		cout << "  avg_time " << (*it).avg_time << " s" << endl;
		cout << "  min_time " << (*it).min_time << " s" << endl;
		cout << "  max_time " << (*it).max_time << " s" << endl;
	}
	return 0;
}