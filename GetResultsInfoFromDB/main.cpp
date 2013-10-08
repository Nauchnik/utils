// Заголовочные файлы
#ifdef _WIN32
#include <my_global.h> // Include this file first to avoid problems
#endif
#include <mysql.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

void ProcessQuery( MYSQL *conn, string str, vector< vector<stringstream *> > &result_vec );
void MakeHTMLfromWU( MYSQL *conn, string wu_id_str );

int main(int argc, char *argv[])
{
	// get result id's from file

	if ( argc < 5 ) {
		cout << "program [assimilator log file] [DB name] [DB login] [DB password]" << endl;
		return 1;
	}
	
	//connection params
	char *host = "localhost";
	char *db   = argv[2];
    char *user = argv[3];
	char *pass = argv[4];
	
	ifstream infile;
	infile.open( argv[1] );
	string str, sat_set_str, wu_id_str, time_str;
	vector<string> wu_id_str_vec;
	
	unsigned found, id_pos, id_len, time_pos, time_len, sat_set_pos, sat_set_len, sat_set_count = 0, cur_pos;
	//cout << "Start reading file" << endl;
	while ( getline(infile, str) ) {
		cur_pos = 0;
		found = str.find( " id " );
		if ( found == string::npos )
			break;
		id_pos = found + 4;
		found = str.find( " ", id_pos );
		id_len = found - id_pos;
		wu_id_str = str.substr( id_pos, id_len );
		for (;;) { // find all SAT sets in string
			found = str.find( " SAT ", cur_pos );
			if ( found == string::npos )
				break;
			sat_set_count++;
			sat_set_pos = found + 5;
			found = str.find( " time ", sat_set_pos );
			sat_set_len = found - sat_set_pos;
			sat_set_str = str.substr( sat_set_pos, sat_set_len );
			cur_pos += sat_set_pos;
			time_pos = found + 6;
			found = str.find( " ", time_pos );
				time_len = ( found != string::npos ) ? found - time_pos : str.length() - found;
			time_str = str.substr( time_pos, time_len);
			/*cout << "id_pos " << id_pos << endl;
			cout << "id_len " << id_len << endl;
			cout << "sat_set_pos " << sat_set_pos << endl;
			cout << "sat_set_len " << sat_set_len << endl;*/
			cout << "sat_set_count # " << sat_set_count << ":" << endl;
			cout << "wu_id " << wu_id_str << endl;
			cout << "sat_set " << sat_set_str << endl;
			cout << "time " << time_str << endl;
			cout << endl;
			wu_id_str_vec.push_back( wu_id_str );
		}
	}
	
	// Дескриптор соединения
	MYSQL *conn;

	// Получаем дескриптор соединения
	conn = mysql_init(NULL);
	if(conn == NULL)
		cerr << "Error: can't create MySQL-descriptor\n";

	// Устанавливаем соединение с базой данных
	if(!mysql_real_connect(conn, host, user, pass, db, 0, NULL, 0))
		cerr << "Error: can't connect to MySQL server\n";

	// Устанавливаем кодировку соединения, чтобы предотвратить
	// искажения русского текста
	//if(mysql_query(conn, "SET NAMES 'utf8'") != 0)
	//   cerr << "Error: can't set character set\n";

	for ( vector<string> ::iterator it = wu_id_str_vec.begin(); it != wu_id_str_vec.end(); it++ )
		MakeHTMLfromWU( conn, *it );

	// Закрываем соединение с сервером базы данных
	mysql_close(conn);

	return 0;
}

void MakeHTMLfromWU( MYSQL *conn, string wu_id_str )
{
	cout << "wu_id_str " << wu_id_str << endl;
	vector< vector<stringstream *> > result_vec;
	string str = "SELECT id FROM result WHERE workunitid=" + wu_id_str;
	cout << str << endl;
	
	ProcessQuery( conn, str, result_vec );

	if ( result_vec.size() == 0 ) {
		cout << "result_vec.size() == 0" << endl;
		return;
	}

	stringstream sstream;
	vector<int> resultid_vec;
	vector<int> userid_vec;
	vector<string> username_vec;
	vector<int> teamid_vec;
	vector<string> teamname_vec;
	vector<string> mod_time_vec;
	unsigned u_val;
	for ( unsigned i = 0; i < result_vec.size(); i++ )
		for ( unsigned j = 0; j < result_vec[i].size(); j++ ) {
			*result_vec[i][j] >> u_val;
			resultid_vec.push_back( u_val );
			delete result_vec[i][j];
		}
	result_vec.clear();

	for ( vector<int>::iterator it = resultid_vec.begin(); it != resultid_vec.end(); it++ ) {
		sstream << "SELECT userid, teamid, mod_time FROM result WHERE validate_state = 1 AND id=" << *it;
		str = sstream.str();
		sstream.clear(); sstream.str("");
		cout << str << endl;
		ProcessQuery( conn, str, result_vec );
		cout << "workunitid " << wu_id_str << endl;
		cout << "resultid " << *it << endl;
		for ( unsigned i = 0; i < result_vec.size(); i++ ) {
			*result_vec[i][0] >> u_val; // get userid
			userid_vec.push_back( u_val );
			cout << "userid " << u_val << endl;
			*result_vec[i][1] >> u_val; // get teamid
			teamid_vec.push_back( u_val );
			cout << "teamid " << u_val << endl;
			str = (*result_vec[i][2]).str();     // get mod_time
			cout << str << endl;
			cout << "mod_time " << str << endl;
			cout << endl;
			mod_time_vec.push_back( str );
			for ( unsigned j = 0; j < result_vec[i].size(); j++ )
				delete result_vec[i][j];
		}
		result_vec.clear();
	}

	// get names of users
	for ( vector<int>::iterator it = userid_vec.begin(); it != userid_vec.end(); it++ ) {
		sstream << "SELECT name FROM user WHERE id=" << *it;
		str = sstream.str();
		sstream.clear(); sstream.str("");
		cout << str << endl;
		ProcessQuery( conn, str, result_vec );

		for ( unsigned i = 0; i < result_vec.size(); i++ ) {
			str = (*result_vec[i][0]).str();
			cout << str << endl;
			username_vec.push_back( str );
			delete result_vec[i][0];
		}
		result_vec.clear();
	}

	// get names of teams
	for ( vector<int>::iterator it = teamid_vec.begin(); it != teamid_vec.end(); it++ ) {
		sstream << "SELECT name FROM team WHERE id=" << *it;
		str = sstream.str();
		sstream.clear(); sstream.str("");
		cout << str << endl;
		ProcessQuery( conn, str, result_vec );

		for ( unsigned i = 0; i < result_vec.size(); i++ ) {
			str = (*result_vec[i][0]).str();
			cout << str << endl;
			teamname_vec.push_back( str );
			delete result_vec[i][0];
		}
		result_vec.clear();
	}

	sstream << "<tr> <td> <b>" << mod_time_vec[0] << " UTC </b> </td>" << endl;
	sstream << "<td> <a href = 'http://sat.isa.ru/pdsat/show_user.php?userid=" << userid_vec[0] << 
			   "'>" << username_vec[0] << "</a> /" << endl;
	sstream << "<a href = 'http://sat.isa.ru/pdsat/show_user.php?userid=" << userid_vec[1] << 
			   "'>" << username_vec[1] << "</a> </td>" << endl;
	sstream << "<td>diag10_2</td>" << endl << "<td> </td>" << endl << "</tr>" << endl;

	cout << sstream.rdbuf();
}

void ProcessQuery( MYSQL *conn, string str, vector< vector<stringstream *> > &result_vec )
{
	// Дескриптор результирующей таблицы
	MYSQL_RES *res;
	// Дескриптор строки
	MYSQL_ROW row;
	int num_fields;

	if(mysql_query(conn, str.c_str()) != 0)
		cerr << "Error: can't execute SQL-query\n";

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