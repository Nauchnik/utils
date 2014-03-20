#ifdef _WIN32
#include <my_global.h> // Include this file first to avoid problems
#endif
#include <mysql.h>

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

void ProcessQuery( MYSQL *conn, string str, vector< vector<stringstream *> > &result_vec );

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
	if ( argc < 4 ) {
		cout << "program [DB name] [DB login] [DB password]" << endl;
		return 1;
	}
	
	//connection params
	char *host = "localhost";
	char *db   = argv[1];
    char *user = argv[2];
	char *pass = argv[3];

	MYSQL *conn = mysql_init(NULL);
	if(conn == NULL)
		cerr << "Error: can't create MySQL-descriptor\n";

	// Устанавливаем соединение с базой данных
	if(!mysql_real_connect(conn, host, user, pass, db, 0, NULL, 0))
		cerr << "Error: can't connect to MySQL server\n";

	vector<string> dir_names = vector<string>();
	getdir( ".", dir_names );
	cout << "dir_names.size() " << dir_names.size() << endl;
	int match_file_size = 36;
	int count = 0;
	for ( unsigned i=0; i < dir_names.size(); i++ )
		if ( dir_names[i].size() == match_file_size ) 
			count++;
	cout << "count " << count << endl;
	vector<string> file_names = vector<string>();
	string sql_string;
	vector< vector<stringstream *> > result_vec;
	int name_db_count = 0;
	int all_name_count = 0;
	int deleted_files = 0;
	string system_str;
	int delete_count = 0;
	
	if ( !count ) {
		for ( unsigned i=0; i < dir_names.size(); i++ ) {
			if ( dir_names.size() > 1 ) {
				cout << "dir_name " << dir_names[i] << " ";
				getdir( dir_names[i], file_names );
				count = 0;
				for ( unsigned j=0; j < file_names.size(); j++ )
					if ( file_names[j].size() == match_file_size ) {
						sql_string = "SELECT id FROM workunit WHERE name LIKE '%" + file_names[j] + "%'";
						if ( all_name_count == 0 )
							cout << "first SQL query: " << sql_string << endl;
						//cout << "before query" << endl;
						ProcessQuery( conn, sql_string, result_vec );
						//cout << "query done" << endl;
						if ( result_vec.size() != 0 )
							name_db_count++;
						else {
							if ( !delete_count )
								system_str = "rm -rf ";
							if ( delete_count < 10 ) {
								system_str += ( "./" + dir_names[i] + "/" + file_names[j] + " " );
								delete_count++;
							}
							else {
								cout << "command : " << system_str << endl;
								system( system_str.c_str() );
								cout << "after command " << endl;
								deleted_files += delete_count;
								cout << "deleted_files " << deleted_files << endl;
								delete_count = 0;
							}
						}
						all_name_count++;
						count++;
						result_vec.clear();
						if ( name_db_count % 100 == 0 )
							cout << "name_db_count " << name_db_count << " from " << all_name_count << endl;
					}
				cout << "file count " << count << endl;
				file_names.clear();
			}
		}
	}
	cout << "name_db_count  " << name_db_count << " from " << all_name_count << endl;
	cout << "*** done" << endl;
	
	mysql_close(conn);
}


void ProcessQuery( MYSQL *conn, string str, vector< vector<stringstream *> > &result_vec )
{
	// Дескриптор результирующей таблицы
	MYSQL_RES *res;
	// Дескриптор строки
	MYSQL_ROW row;
	int num_fields;

	if(mysql_query(conn, str.c_str()) != 0) {
		cerr << "Error: can't execute SQL-query\n";
		cerr << "query: " << str << endl;
		exit(1);
	}

	// Получаем дескриптор результирующей таблицы
	res = mysql_store_result(conn);

	if( res == NULL ) 
		cerr << "Error: can't get the result description\n";

	num_fields = mysql_num_fields(res);
	stringstream *sstream_p;
	vector<stringstream *> result_data;

	// Если имеется хотя бы одна запись - выводим список каталогов
	if ( mysql_num_rows(res) > 0 ) {
		// В цикле перебираем все записи результирующей таблицы
		while((row = mysql_fetch_row(res)) != NULL) {
			for( int i = 0; i < num_fields; i++ ) {
				//cout << row[i] << endl;
				sstream_p = new stringstream();
				*sstream_p << row[i]; // get value
				result_data.push_back( sstream_p );
			}
			result_vec.push_back( result_data );
			result_data.clear();
		}
	}

	// Освобождаем память, занятую результирующей таблицей
	mysql_free_result(res);
}