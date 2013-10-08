#ifndef hash_process_h
#define hash_process_h

#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <ctime>
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>
#include <hash_map>
#ifdef _WIN32
#include "pstdint.h"
#include "dirent.h"
#else
#include <stdint.h> 
#include <dirent.h>
#endif

using namespace std;

#ifdef _WIN32
using namespace stdext;
#else
using namespace __gnu_cxx;
#endif

const int HASH_INIT             = 1000;
const int PRINT_EVERY_CONFLICT  = 1000000;
const int WRITE_EVERY_CLAUSE    = 1000000;
const int OFSTREAM_COUNT        = 768;
const int MAX_CLAUSE_STRING_LEN = 65536;
const int MAX_HASH_TAB_SIZE     = 30000000;
const int MAX_ENTRY_VALUE       = 2048;

static inline double cpuTime( void ) 
{ return ( double )clock( ) / CLOCKS_PER_SEC; }

// store vector
struct Hash_tab_value
{
	vector<short> lit_vec;
	unsigned short entry_count;
};

// store 2nd hash instead of vector
struct Cut_hash_tab_value
{
	uint32_t second_hash_val;
	unsigned short entry_count;
};

struct linesByLen_value
{
	int vec_size;
	int entry_count;
};

struct lenByLines_value
{
	int string_len;
	int vec_size;
};

class Hash_process
{
public:
	Hash_process(void);
	~Hash_process(void);

	string input_file_name;
	string stat_file_name;
	string split_stat_file_name;

	bool IsNoVecMode;
	int GetStartStatMode;
	int SplitInpFileMode;
	int repeat_count_arr_len;
	int freq_arr_len;
	int* repeat_count_arr;
	int* freq_arr;

	int clauses_count;
	int repeat_count;
	int collisions_count;
	int processed_clauses;

	double start_time;

	hash_multimap<uint32_t, Hash_tab_value> hash_tab;
	typedef hash_multimap<uint32_t, Hash_tab_value>::iterator hash_tab_iterator;
	hash_tab_iterator tab_i;
	pair<hash_tab_iterator, hash_tab_iterator> pos;

	hash_multimap<uint32_t, Cut_hash_tab_value> cut_hash_tab;

	// get hash value from vector
	uint32_t hashvec( vector<short> k, size_t length, uint32_t initval );
	
	vector<short> GetVecFromString( string clause_string );
	bool ReadInpFile( );
	void MakeHashTabFromFile( string file_name );
	// standart mode - store vectors ans their hash values
	bool MakeHashTabVec( uint32_t hash_val, vector<short> lit_vec, bool &IsRepeat );  
	// approximate mode - store 2ns hash value instead of vector
	bool MakeHashTabNoVec( uint32_t hash_val, vector<short> lit_vec );
	void PrintCurrentStat( int current_processed_clauses );
	void PrintFinalStat( );
	void InitRepeatCount( );
	void GetStartStat( );

	void TestByBruteForce( );
};

#endif
