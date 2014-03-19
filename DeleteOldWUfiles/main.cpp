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
	vector<string> dir_names = vector<string>();
	getdir( ".", dir_names );
	cout << "dir_names.size() " << dir_names.size() << endl;
	int min_file_size = 20;
	int count = 0;
	for ( unsigned i=0; i < dir_names.size(); i++ )
		if ( dir_names[i].size() > min_file_size ) 
			count++;
	cout << "count " << count << endl;
	vector<string> file_names = vector<string>();
	if ( !count ) {
		for ( unsigned i=0; i < dir_names.size(); i++ ) {
			if ( dir_names.size() > 1 ) {
				cout << "dir_name " << dir_names[i];
				getdir( dir_names[i], file_names );
				count = 0;
				for ( unsigned j=0; j < file_names.size(); j++ )
					if ( file_names[j].size() > min_file_size ) 
						count++;
				cout << "count " << count << endl;
				file_names.clear();
			}
		}
	}
}