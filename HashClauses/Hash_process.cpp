#include "Hash_process.h"

Hash_process :: Hash_process( void ) :
	IsNoVecMode          ( false ),
	GetStartStatMode     ( 0 ),
	SplitInpFileMode     ( 0 ),
	clauses_count        ( 0 ),
	repeat_count         ( 0 ),
	collisions_count     ( 0 ),
	processed_clauses    ( 0 ),
	repeat_count_arr_len ( 0 ),
	freq_arr_len         ( 0 )
{ }

Hash_process :: ~Hash_process( void )
{ }

// rot and mix are functions by Bob Jenkins
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

/*-------------------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly
*/
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

/*-------------------------------------------------------------------------------
final -- final mixing of 3 32-bit values (a,b,c) into c
*/

#define final(a,b,c) \
{ \
	c ^= b; c -= rot(b,14); \
	a ^= c; a -= rot(c,11); \
	b ^= a; b -= rot(a,25); \
	c ^= b; c -= rot(b,16); \
	a ^= c; a -= rot(c,4);  \
	b ^= a; b -= rot(a,14); \
	c ^= b; c -= rot(b,24); \
}

//-------------------------------------------------------------------------------
uint32_t  Hash_process :: hashvec(
vector<short> k,       /* the key, a vector of int values */
size_t        length,  /* the length of the key, in uint32_ts */
uint32_t      initval) /* the previous hash, or an arbitrary value */
{
// analog of Bob Jenkins' hashword
	uint32_t a,b,c;

	/* Set up the internal state */
	a = b = c = 0xdeadbeef + (((uint32_t)length)<<2) + initval;

	/*------------------------------------------------- handle most of the key */
	int i = 0;
	while (length > 3)
	{
		a += ( uint32_t )k[i + 0];
		b += ( uint32_t )k[i + 1];
		c += ( uint32_t )k[i + 2];
		mix(a,b,c);
		length -= 3;
		i += 3;
	}

	 /*------------------------------------------- handle the last 3 uint32_t's */
	switch( length )                     /* all the case statements fall through */
	{ 
		case 3 : c+=( uint32_t )k[i + 2];
		case 2 : b+=( uint32_t )k[i + 1];
		case 1 : a+=( uint32_t )k[i + 0];
			final(a,b,c);
		case 0:     /* case 0: nothing left to add */
		break;
	}
	/*------------------------------------------------------ report the result */
	return c;
}

//---------------------------------------------------------
bool SortPredicate( short i, short j ) 
{ return ( i < j ); }

//-------------------------------------------------------------------------------
bool Hash_process :: MakeHashTabVec( uint32_t hash_val, vector<short> lit_vec, bool &IsRepeat )
{
	hash_multimap<uint32_t, Hash_tab_value> :: iterator p;
	Hash_tab_value htv;
	bool IsVecExists;
	IsRepeat = false; // default

	switch ( hash_tab.count( hash_val ) ) // find hash value in tab
	{
		case 0: // 0 found, add new pair to hash table
		{
			htv.lit_vec = lit_vec;
			htv.entry_count = 1;
			hash_tab.insert( pair<uint32_t, Hash_tab_value>( hash_val, htv ) );
			break;
		}
		default: // at least 1 found
		{
			pos = hash_tab.equal_range( hash_val );
			IsVecExists = false;
			for (; pos.first != pos.second; pos.first++ ) { // for every collision
				if ( lit_vec == pos.first->second.lit_vec ) { // increment counter
					IsVecExists = true;
					(pos.first->second.entry_count)++;
					repeat_count++;
					IsRepeat = true;
					break;
				}
			}
			if ( !IsVecExists ) { // add new value to hash table
				collisions_count++;
				htv.lit_vec = lit_vec;
				htv.entry_count = 1;
				hash_tab.insert( pair<uint32_t, Hash_tab_value>( hash_val, htv) );
			}
			break;
		}
	} // switch 

	return true;
}

//-------------------------------------------------------------------------------
bool Hash_process :: MakeHashTabNoVec( uint32_t hash_val, vector<short> lit_vec )
{
	bool IsVecExists;
	uint32_t second_hash_val;
	hash_multimap<uint32_t, Cut_hash_tab_value> :: iterator cut_p;
	Cut_hash_tab_value cut_htv;
	typedef hash_multimap<uint32_t, Cut_hash_tab_value>::iterator cut_iterator;
	cut_iterator cut_tab_i;
	pair<cut_iterator, cut_iterator> cut_pos;

	switch ( cut_hash_tab.count( hash_val ) ) { // find hash value in tab
		case 0: // 0 found, add new pair to hash table
		{
			cut_htv.second_hash_val = hashvec( lit_vec, lit_vec.size( ), HASH_INIT ); // get hash value
			cut_htv.entry_count = 1;
			cut_hash_tab.insert( pair<uint32_t, Cut_hash_tab_value>( hash_val, cut_htv ) );
			break;
		}
		default: // at least 1 found
		{
			second_hash_val = hashvec( lit_vec, lit_vec.size( ), HASH_INIT );
			cut_pos = cut_hash_tab.equal_range( hash_val );
			IsVecExists = false;
			for (; cut_pos.first != cut_pos.second; cut_pos.first++ ) { // for every collision
				if ( second_hash_val == cut_pos.first->second.second_hash_val ) { // increment counter
					IsVecExists = true;
					(cut_pos.first->second.entry_count)++;
					/*if ( cut_pos.first->second.entry_count == 255)
						cout << "*** \n\n\n cut_pos.first->second.entry_count == 255 \n\n\n";*/
					repeat_count++;
								//
					break;
				}
			}
			if ( !IsVecExists ) { // add new value to hash table
				collisions_count++;
				cut_htv.second_hash_val = second_hash_val;
				cut_htv.entry_count = 1;
				cut_hash_tab.insert( pair<uint32_t, Cut_hash_tab_value>( hash_val, cut_htv ) );
			}
		break;
		}
	} // switch 

	return true;
}

//-------------------------------------------------------------------------------
void Hash_process :: PrintFinalStat( )
{
	int i;
	int freq_arr_len = 1024;
	int current_processed_clauses;
	stringstream sstream;
	double percent_val;
	ofstream split_stat_file;
	cout << "\n Start PrintHashStat";
	
	cout << endl << "repeat_count_arr_len is " << repeat_count_arr_len;
	for ( i = 1; i < repeat_count_arr_len; i++ )
		repeat_count_arr[i] += repeat_count_arr[i - 1];

	split_stat_file.open( split_stat_file_name.c_str( ), ios :: app );
	for ( i = 0; i < repeat_count_arr_len; i++ ) {
		if ( i < repeat_count_arr_len - 1 )
			current_processed_clauses = 1000000 * ( i + 1);
		else
			current_processed_clauses = clauses_count;
		percent_val = double( repeat_count_arr[i] ) / double( current_processed_clauses / 100 );
		//cout << endl << endl;
		//cout << endl << " Repeat count " << repeat_count_arr[i] << " from " << current_processed_clauses;
		//cout << endl << " % of repeat is " << percent_val;
		
		sstream << current_processed_clauses << " " << percent_val << "\n";
		split_stat_file << sstream.rdbuf( );
		sstream.str( "" );
	}
	
	delete[] repeat_count_arr;
	split_stat_file.close( );
	
	int last_zero_index = freq_arr_len - 1;
	while ( freq_arr[last_zero_index] == 0 )
		last_zero_index--;
	last_zero_index++;

	ofstream freq_file;
	freq_file.open( "freq_file", ios :: out );

	cout << endl << endl << " Frequency";
	for( i = 1; i < last_zero_index; i++ ) {
		cout << endl << " entry_count " << i << " freq_count " << freq_arr[i];
		sstream << i << " " << freq_arr[i] << endl;
		freq_file << sstream.rdbuf( );
		sstream.str( "" );
	}

	freq_file.close( );
	delete[] freq_arr;
	/*
	long long int mid_clauses_len = 0;
	// get stat about mid len of clauses
	for ( tab_i = hash_tab.begin( ); tab_i != hash_tab.end( ); tab_i++ )
	{
		pos = hash_tab.equal_range( hash_val );
		for (; pos.first != pos.second; pos.first++ ) // for every collision
			mid_clauses_len += ( long long int )pos.first->second.lit_vec.size( ); // increment counter
	}
	mid_clauses_len = mid_clauses_len / clauses_count;
	cout << endl << " mid_clauses_len is " << mid_clauses_len;*/
}

//-------------------------------------------------------------------------------
void Hash_process :: TestByBruteForce( )
{
// check repeat count
	//lit_vec_arr.push_back( lit_vec );
	/*vector<vector<short>> lit_vec_arr;
	unsigned int i, j; 
	int test_repreat_count = 0;
	for ( i = 0; i < lit_vec_arr.size( ); i++ )
	{	
		for ( j = 0; j < lit_vec_arr.size( ); j++ )
			if ( ( lit_vec_arr[i].size( ) > 0 ) && ( lit_vec_arr[j].size( ) > 0 ) && 
				  ( i != j ) && ( lit_vec_arr[i] == lit_vec_arr[j] ) )
			{
				test_repreat_count++;
				lit_vec_arr[j].clear( );
			}
		if ( i % 1000 == 0 )
			cout << "\n i is " << i;
	}
	cout << "\n test_repreat_count is " << test_repreat_count;*/
}

//-------------------------------------------------------------------------------
void Hash_process :: PrintCurrentStat( int current_processed_clauses )
{
	ofstream stat_file;
	stringstream stream_val;
	double percent_val;

	double end_time = cpuTime( ) - start_time;
	percent_val = double( repeat_count ) / double( current_processed_clauses / 100 );
	cout << endl;
	cout << endl << " Processed time is "   << end_time; 
	cout << endl << " Processed "           << current_processed_clauses << " from " << clauses_count;
	cout << endl << " Repeat count "        << repeat_count;
	cout << endl << " % of repeat is "      << percent_val; 
	cout << endl << " Collisions count is " << collisions_count;
	stream_val << current_processed_clauses << " " << percent_val << "\n";
	stat_file.open( stat_file_name.c_str( ), ios::app );
	stat_file << stream_val.rdbuf( );
	stat_file.close( );
	stream_val.str( "" );
}

//-------------------------------------------------------------------------------
vector<short> Hash_process :: GetVecFromString( string clause_string )
{
	vector<short> lit_vec;
	string word_str;
	int i, k, string_end;
	//int current_lit_count = 0;
	clause_string = " " + clause_string; // add space to line for correct work of parser

	string_end = clause_string.find( " 0" );

	if ( string_end == -1 )
	{ 
		cout << "Warning. string with no '#' and no '0' on the end. Skipped";
		lit_vec.clear( );
		return lit_vec;
	}
	
	for ( i = 0; i < string_end; i++ )
	{
		if ( ( clause_string[i] == ' ' ) && ( clause_string[i + 1] != ' ' ) && 
			 ( clause_string[i + 1] != '0' ) )
		{
			word_str = ""; // init string for cuttenr word from string
			k = i + 1; // k = index of first symbol in current word
			do
			{
				word_str += clause_string[k];
				k++;
			} while ( clause_string[k] != ' ' );

			//current_lit_count++;
			//clause[current_lit_count - 1] = atoi( word_str.c_str( ) );
			lit_vec.push_back( atoi( word_str.c_str( ) ) );
		}
	} // for ( i = 0; i < line_str.length( ) - 1; i++ )
	return lit_vec;
}

//-------------------------------------------------------------------------------
void Hash_process :: MakeHashTabFromFile( string file_name )
{
// file_name can differ from input_file_name - in the case of file splitting
	ifstream input_file;
	string clause_string;
	vector<short> lit_vec;
	uint32_t hash_val;
	bool IsRepeat;
	int string_num_ind, string_num, million_num;
	string str;
	//int current_processed_clauses = 0;

	input_file.open( file_name.c_str( ) );
	while( getline( input_file, clause_string ) ) { // read string with clause
		lit_vec = GetVecFromString( clause_string );
		if ( !lit_vec.size( ) ) 
		{ cout << "\n WARNING. skip empty string"; continue; } // skip empty string

		sort( lit_vec.begin( ), lit_vec.end( ), SortPredicate );  // sort vector
		hash_val = hashvec( lit_vec, lit_vec.size( ), 0 ); // get hash value

		//cout << "\n hash_val is " << hash_val;
		if ( IsNoVecMode ) // Use cut hash - with additional hash value instead vector
			MakeHashTabNoVec( hash_val, lit_vec );
		else
			MakeHashTabVec( hash_val, lit_vec, IsRepeat ); // store vector and hash

		if ( ( IsRepeat ) && ( clause_string.find( "#" ) != -1 ) ) // if lit_vec is repeat then count it
		{
			string_num_ind = clause_string.find( "#" ) + 1;

			if ( string_num_ind > 0 )
			{
				str = "";
				while( string_num_ind < ( int )clause_string.length( ) ) {
					str += clause_string[string_num_ind];
					string_num_ind++;
				}
				string_num = atoi( str.c_str( ) );
			}
			//cout << endl << "string_num is " << string_num;
			million_num = ( int )( floor( ( double )string_num / 1000000 ) );
			//cout << endl << "million_num is " << million_num;
			repeat_count_arr[million_num]++;
		}

		lit_vec.clear( );
		processed_clauses++;
		if ( processed_clauses % PRINT_EVERY_CONFLICT == 0 ) // ptint current stat
			PrintCurrentStat( processed_clauses );
	}
	input_file.close( );

	PrintCurrentStat( processed_clauses ); // ptint current stat

	for ( tab_i = hash_tab.begin( ); tab_i != hash_tab.end( ); tab_i++ )
	{
		hash_val = tab_i -> first;
		pos = hash_tab.equal_range( hash_val );
		for (; pos.first != pos.second; pos.first++ ) // for every collision
			freq_arr[pos.first->second.entry_count]++; // increment counter of repeats
	}

	if ( SplitInpFileMode ) {
		cout << endl << " *** processed file with name " << file_name;
		//cout << endl << " *** total processed_clauses is " << processed_clauses;
	}
}

void Hash_process :: InitRepeatCount( )
{
	// get count of millions in clauses_count
	repeat_count_arr_len = int( ceil( double ( clauses_count ) / 1000000  ) );
	cout << endl << "*** repeat_count_arr_len is " << repeat_count_arr_len;
	// array of counters
	repeat_count_arr = new int[repeat_count_arr_len];
	for( int i = 0; i < repeat_count_arr_len; i++ )
		repeat_count_arr[i] = 0;
}

void Hash_process :: GetStartStat( )
{
	ifstream input_file;
	ofstream cur_file;
	string clause_string, cur_file_name;
	stringstream sstream;
	string str_len_file_name = "str_len_file";
	vector<short> lit_vec;
	int lit_vec_size;
	bool IsValExists;
	uint32_t curLen;
	// for (GetStartStatMode == 1)
	map<uint32_t, uint32_t> ShortLinesByLen; // key is string length
	typedef map<uint32_t, uint32_t> :: iterator ShortLinesByLenIt;
	ShortLinesByLenIt slbl_i;
	map<uint32_t, uint32_t> ShortLenByLines; // key is string length
	map<uint32_t, uint32_t> :: iterator ShortLenByLinesIt;
	// for (GetStartStatMode == 2)
	multimap<uint32_t, linesByLen_value> linesByLen; // key is string length
	typedef multimap<uint32_t, linesByLen_value>::iterator linesByLenIt;
	linesByLenIt lbl_i;
	pair<linesByLenIt, linesByLenIt> linesByLen_pos;
	linesByLen_value line_bl_val;
	multimap<uint32_t, lenByLines_value> lenByLines;
	multimap<uint32_t, lenByLines_value> :: iterator lenByLinesIt;
	lenByLines_value len_bl_val;
	//pair<lenByLinesIt, lenByLinesIt> lenByLines_pos;
	// map for storing opened files
	map<string, ofstream*> strByOfstream;
	map<string, ofstream*> :: iterator strByOfstreamIt;

	input_file.open( input_file_name.c_str( ) );
	if ( GetStartStatMode == 1 )
	{
		// if ( IsGetStartStat ) - slow way, oount all stat
		while( getline( input_file, clause_string ) )
		{
			lit_vec = GetVecFromString( clause_string );
			if ( !lit_vec.size( ) ) 
			{ cout << "\n WARNING. skip empty string"; continue; }// skip empty string
			else clauses_count++;

			curLen = clause_string.length( );
			slbl_i = ShortLinesByLen.find( curLen );
			if ( slbl_i == ShortLinesByLen.end( ) )
				ShortLinesByLen.insert( pair<uint32_t,uint32_t>( curLen, 1 ) ); //add new, count == 1
			else
				slbl_i->second++; // increment count of vectors with such len
		}
		cout << endl << "string length : string count";
		for( slbl_i = ShortLinesByLen.begin( ); slbl_i != ShortLinesByLen.end( ); slbl_i++ )
		{
			cout << endl << slbl_i->first << " : " << slbl_i->second;
			ShortLenByLines.insert( pair<uint32_t,uint32_t>( slbl_i->second, slbl_i->first ) );
		}
		cout << endl << endl << "string count : string length";
		for( ShortLenByLinesIt = ShortLenByLines.begin( ); ShortLenByLinesIt != ShortLenByLines.end( ); ShortLenByLinesIt++ )
			cout << endl << ShortLenByLinesIt->first << " : " << ShortLenByLinesIt->second;
		
		ShortLinesByLen.clear( );
		ShortLenByLines.clear( );
		cout << endl;
		cout << endl << "ShortLinesByLen.size( ) is " << ShortLinesByLen.size( );
	}
	else if ( GetStartStatMode == 2 )
	{
		// if ( IsGetStartStat ) - slow way, oount all stat
		while( getline( input_file, clause_string ) )
		{
			lit_vec = GetVecFromString( clause_string );
			if ( !lit_vec.size( ) ) 
			{ cout << "\n WARNING. skip empty string"; continue; }// skip empty string
			else clauses_count++;

			curLen = clause_string.length( );
			lit_vec = GetVecFromString( clause_string );
			
			if ( !lit_vec.size( ) ) 
				{ cout << "\n WARNING. skip empty string"; continue; } // skip empty string

			lit_vec_size = lit_vec.size( );
			
			switch ( linesByLen.count( curLen ) ) // find hash value in tab
			{
				case 0: // 0 found, add new pair to table
				{
					line_bl_val.vec_size = lit_vec_size;
					line_bl_val.entry_count = 1;
					linesByLen.insert( pair<uint32_t, linesByLen_value>( curLen, line_bl_val ) );
					break;
				}
				default: // at least 1 found
				{
					linesByLen_pos = linesByLen.equal_range( curLen );
					IsValExists = false;
					for (; linesByLen_pos.first != linesByLen_pos.second; linesByLen_pos.first++ ) // for every val
					{
						if ( lit_vec_size == linesByLen_pos.first->second.vec_size ) // increment counter
						{
							IsValExists = true;
							(linesByLen_pos.first->second.entry_count)++;
							break;
						}
					}
					if ( !IsValExists ) // add new value to hash table
					{
						line_bl_val.vec_size = lit_vec_size;
						line_bl_val.entry_count = 1;
						linesByLen.insert( pair<uint32_t, linesByLen_value>( curLen, line_bl_val) );
					}
					break;
				}
			} // switch 
		}
		cout << endl << "string length : vec_size : string count";
		for( lbl_i = linesByLen.begin( ); lbl_i != linesByLen.end( ); lbl_i++ )	
		{
			cout << endl << lbl_i->first << " : " << lbl_i->second.vec_size << " : " << lbl_i->second.entry_count;
			len_bl_val.string_len = lbl_i->first;
			len_bl_val.vec_size   = lbl_i->second.vec_size;
			lenByLines.insert( pair<uint32_t, lenByLines_value>( lbl_i->second.entry_count, len_bl_val ) );
		}

		cout << endl;
		cout << endl << "string count : string length : vec_size";
		for( lenByLinesIt = lenByLines.begin( ); lenByLinesIt != lenByLines.end( ); lenByLinesIt++ )
			cout << endl << lenByLinesIt->first << " : " << lenByLinesIt->second.string_len 
						 << " : " << lenByLinesIt->second.vec_size;
		cout << endl << "lenByLines.size( ) is " << lenByLines.size( );
		linesByLen.clear( );
		lenByLines.clear( );
	}
	input_file.close( );
}

//-------------------------------------------------------------------------------
bool Hash_process :: ReadInpFile( )
{
// Read input files and optionally split it into files with small size
	ifstream input_file;
	ofstream cur_file;
	stringstream *sstream_arr;
	//ofstream *ofstr_arr;
	string clause_string, cur_file_name, system_str;
	stringstream sstream;
	string str_len_file_name = "str_len_file";
	vector<short> lit_vec;
	int lit_vec_size, write_batch_count, max_str_len = 0;
	uint32_t curLen;
	cout << endl << endl << "*** Start of ReadInpFile ***";

	freq_arr_len = MAX_ENTRY_VALUE;
	freq_arr = new int[freq_arr_len];
	for( int i = 0; i < freq_arr_len; i++ )
		freq_arr[i] = 0;

	if ( SplitInpFileMode == 3 ) { // if files exist
		ifstream str_len_file( str_len_file_name.c_str( ) );
		getline( str_len_file, clause_string );
		clauses_count = atoi( clause_string.c_str( ) );
		cout << endl << endl << "** clauses_count was read " << clauses_count;
		str_len_file.close( );
		InitRepeatCount( );
		return true;
	}

	// delete zplit files if such exist
	#ifdef _WIN32
		system_str = "del zplit*";
	#else
		system_str = "rm -r zplit*";
	#endif
	system( system_str.c_str( ) );
	cout << endl << "Correct delete of files";
	clauses_count = 0;

	input_file.open( input_file_name.c_str( ) );

	if ( SplitInpFileMode == 1 ) { // fast way - only count strings
		// read max len to allocate arrays
		while( getline( input_file, clause_string ) ) {
			clauses_count++;
			if ( max_str_len < ( int )clause_string.length( ) ) {
				lit_vec = GetVecFromString( clause_string );
				if ( !lit_vec.size( ) ) { // skip empty string
					cout << "\n WARNING. skip wrong string # " << clauses_count << " String is " << clause_string;
					continue; 
				}
				if ( clause_string.length( ) > MAX_CLAUSE_STRING_LEN ) {
					cout << "\n WARNING. clause_string.length( ) > MAX_CLAUSE_STRING_LEN. skip wrong string # " << clauses_count 
						 << " clause_string.length( )is " << clause_string.length( );
					continue; 
				}

				max_str_len = ( int )clause_string.length( );
				cout << endl << endl << "new max_str_len "  << max_str_len;
				cout << endl << "clauses_count is " << clauses_count;
				cout << endl << "clause_string is " << clause_string;
			}
		}

		input_file.close( );
		input_file.clear( );
		input_file.open( input_file_name.c_str( ) );
		cout << endl << "max_str_len is " << max_str_len;
		//ofstr_arr   = new ofstream[max_str_len];
		sstream_arr = new stringstream[max_str_len];
	}

	ofstream ofstr;
	write_batch_count = 0;
	clauses_count = 0;
	//cout << endl << "1";
	while( getline( input_file, clause_string ) ) {
		lit_vec = GetVecFromString( clause_string );
		//cout << endl << clause_string;
		if ( !lit_vec.size( ) ) { // skip empty string
			cout << "\n WARNING. skip wrong string # " << clauses_count << " String is " << clause_string;
			continue; 
		}
		else clauses_count++;

		//cout << endl << "clauses_count is " << clauses_count;

		if ( SplitInpFileMode == 1 ) { // fast split large inp file into a set of files
			curLen = clause_string.length( );
			if ( (int)curLen > max_str_len ) {
				cout << endl << "Error. curLen > max_str_len, i.e. " << curLen  << " < " << max_str_len;
				return false;
			}
			//cout << endl << "curLen " << curLen;
			sstream_arr[curLen - 1] << clause_string << " #" << clauses_count << "\n";
			//cout << endl << "added to sstream_arr ";
			if ( clauses_count % WRITE_EVERY_CLAUSE == 0 ) {
				write_batch_count++;
				cout << endl << "write batch # " << write_batch_count << " with " 
					 << WRITE_EVERY_CLAUSE << " clauses";
				for ( int i = 0; i < max_str_len; i++ )
				{
					if ( sstream_arr[i].str( ).size( ) > 0 )
					{
						sstream << "zplit_" << i + 1;
						cur_file_name = sstream.str( );
						sstream.str( "" ); // clear stringstream
						ofstr.open( cur_file_name.c_str( ), ios :: app );
						ofstr << sstream_arr[i].rdbuf( );
						sstream_arr[i].str( "" );
						ofstr.close( );
					}
				}
			} // if ( clauses_count % WRITE_EVERY_CLAUSE == 0 )
		} // if ( SplitInpFileMode == 1 )
		else if ( SplitInpFileMode == 2 ) { // slow split large inp file into a set of files
			lit_vec = GetVecFromString( clause_string );
			if ( !lit_vec.size( ) ) 
			{ cout << "\n WARNING. skip empty string"; continue; }// skip empty string
			else clauses_count++;
			lit_vec_size = lit_vec.size( );
			curLen = clause_string.length( );
			sstream << "zplit_" << curLen << "_" << lit_vec_size;
			cur_file_name = sstream.str( );
			sstream.str( "" ); // clear stringstream
			cur_file.open( cur_file_name.c_str( ), ios :: app );
			clause_string += "\n";
			cur_file.write( clause_string.c_str( ), clause_string.length( ) );
			cur_file.close( );
		}
	}
	// write last batch of clauses to files
	for ( int i = 0; i < max_str_len; i++ ) {
		if ( sstream_arr[i].str( ).size( ) > 0 ) {
			sstream << "zplit_" << i + 1;
			cur_file_name = sstream.str( );
			sstream.str( "" ); // clear stringstream
			ofstr.open( cur_file_name.c_str( ), ios :: app );
			ofstr << sstream_arr[i].rdbuf( );
			sstream_arr[i].str( "" );
			ofstr.close( );
		}
	}
	//for ( int i = 0; i < max_str_len; i ++ )
	//	sstream_arr[i].str( "" );
	delete[] sstream_arr;
	//delete[] ofstr_arr;
	// write count of lines in input file
	ofstream str_len_file( str_len_file_name.c_str( ) );
	str_len_file << clauses_count;
	str_len_file.close( );
	//strByOfstream.clear( );
	input_file.close( );
	// get count of millions in clauses_count
	InitRepeatCount( );
	cout << endl << "\n clauses_count is "  << clauses_count;
	cout << endl << "\n Processed time is " << cpuTime( ) - start_time;

	return true;
}