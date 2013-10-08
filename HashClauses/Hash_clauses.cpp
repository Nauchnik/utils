//
#include <mpi.h>
#include "Hash_process.h"

#pragma warning( disable : 4996 )

//---------------------------------------------------------
void TestSolve( );

//---------------------------------------------------------
void WriteUsage( );

//---------------------------------------------------------
bool hasPrefix_String( string str, string prefix, string &value );

//---------------------------------------------------------
bool GetInputFlags( int &argc, char **&argv, bool &IsNoVecMode, int &GetStartStatMode, 
				    int &SplitInpFileMode );

//---------------------------------------------------------
int getdir( string dir, vector<string> &files );

//---------------------------------------------------------
int main( int argc, char** argv )
{
	int strLen, SplitInpFileMode, GetStartStatMode;
	string input_file_name;
	double start_time;
	bool IsNoVecMode;
	stringstream sstream;
	Hash_process hash_p;

	hash_p.stat_file_name = "stat";
	hash_p.split_stat_file_name = "split_stat";

	//TestSolve( ); // Debug

	int corecount = 0, rank = 0;

	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &corecount );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	if ( rank == 0 ) {
		start_time = cpuTime( );

		if ( !GetInputFlags( argc, argv, IsNoVecMode, GetStartStatMode, SplitInpFileMode ) )
		{ cout << "\n Error in GetInputFlags"; return 1; }

		sstream << argv[1];
		input_file_name = sstream.str( );

		cout << "\n input_file_name is "  << input_file_name;
		cout << "\n NoVecMode is "        << IsNoVecMode;
		cout << "\n GetStartStatMode is " << GetStartStatMode;
		cout << "\n SplitInpFileMode is " << SplitInpFileMode;

		hash_p.IsNoVecMode      = IsNoVecMode;
		hash_p.GetStartStatMode = GetStartStatMode;
		hash_p.SplitInpFileMode = SplitInpFileMode;
		hash_p.input_file_name  = input_file_name;
		hash_p.start_time       = start_time;
		
		if  ( !( hash_p.ReadInpFile( ) ) )
		{ cout << endl << endl << "*** Error in ReadInpFile. exit ***"; return 1; }
		cout << endl << endl << "*** End of ReadInpFile ***";

		if ( hash_p.GetStartStatMode )
			hash_p.GetStartStat( );

		if ( !SplitInpFileMode ) { // if only 1 file
			cout << endl << "Start work with one files";
			hash_p.MakeHashTabFromFile( input_file_name );
		}
		else if ( ( SplitInpFileMode == 1 ) || ( SplitInpFileMode == 3 ) ) { // many subfiles
			string dir = string(".");
			vector<string> files = vector<string>();
			getdir( dir, files );
			
			cout << endl << "Start work with subfiles";
			for ( unsigned int i = 0; i < files.size( ); i++ ) {
				if ( sscanf( files[i].c_str( ), "zplit_%d", &strLen ) ) {
					hash_p.MakeHashTabFromFile( files[i] );
					hash_p.hash_tab.clear( ); // clear hash_tab after every file
					cout << endl << "*** hash_tab was cleared";
				}
			}
			cout << endl << "End work with subfiles";
		}
		
		cout << endl << "Start of PrintFinalStat";
		hash_p.PrintFinalStat( ); // print final stat
		cout << endl << "End of PrintFinalStat";

		return 0;
	}
}

//---------------------------------------------------------
void WriteUsage( )
{
// Write info about usage
	std :: cout << 
	"\n USAGE: Hash_clauses [options] <clauses_file>"
	"\n options::"
	"\n   -NoVecMode - mode with storing 2nd hash instead of vec"
	"\n   -GetStartStatMode - get stat about vectors in input file"
	"\n      0 - default, no stat"
	"\n      1 - Stat about len of strings"
	"\n      2 - Stat about len of strings and size of vecs"
	"\n   -SplitInpFIleMode - mode of splitting of input file"
	"\n      0 - default, no split, direct process of input file"
	"\n      1 - split by lens of strings"
	"\n      2 - split by lens of strings and size of vecs"
	"\n      3 - use files in dir - split by lens of strings"
	<< endl;
}

bool hasPrefix_String( string str, string prefix, string &value )
{
	int found = str.find( prefix );
	if ( found != -1 ) {
		value = str.substr( found + prefix.length( ) );
		return true;
	}
	else return false;
}

//---------------------------------------------------------
void TestSolve( )
{
	int strLen, vecSize;
	string input_file_name;
	double start_time;
	bool IsNoVecMode;
	int GetStartStatMode,
		SplitInpFileMode;
	stringstream sstream;
	Hash_process hash_p;

	hash_p.stat_file_name = "stat";
	hash_p.split_stat_file_name = "split_stat";

	int argc = 4;
	char** argv = new char*[argc];
	argv[0] = "Hash_clauses";
	argv[1] = "-GetStartStat=0";
	argv[2] = "-SplitInpFileMode=1";
	argv[3] = "clauses0.cnf";
	//argv[2] = "zplit_100";
	
	if ( !GetInputFlags( argc, argv, IsNoVecMode, GetStartStatMode, SplitInpFileMode ) )
	{ cout << "\n Erroe in GetInputFlags"; }
	
	start_time = cpuTime( );

	sstream << argv[1];
	input_file_name = sstream.str( );

	cout << "\n input_file_name is "  << input_file_name;
	cout << "\n NoVecMode is "        << IsNoVecMode;
	cout << "\n GetStartStatMode is " << GetStartStatMode;
	cout << "\n SplitInpFileMode is " << SplitInpFileMode;

	hash_p.IsNoVecMode      = IsNoVecMode;
	hash_p.GetStartStatMode = GetStartStatMode;
	hash_p.SplitInpFileMode = SplitInpFileMode;
	hash_p.input_file_name  = input_file_name;
	hash_p.start_time       = start_time;
		
	//hash_p.clauses_count = 1000;

	cout << endl << "Start of ReadInpFile";
	hash_p.ReadInpFile( );
	cout << endl << "End of ReadInpFile";

	if ( !SplitInpFileMode ) // only 1 file
		hash_p.MakeHashTabFromFile( input_file_name );
	else { // many small files
		string dir = string(".");
		vector<string> files = vector<string>( );
		getdir(dir, files);
			
		for ( unsigned int i = 0; i < files.size( ); i++ ) {
			if ( sscanf( files[i].c_str( ), "zplit_%d_%d", &strLen, &vecSize ) )
				hash_p.MakeHashTabFromFile( files[i] );
		}
	}
		
	cout << endl << "Start of PrintFinalStat";
	hash_p.PrintFinalStat( ); // print final stat
	cout << endl << "End of PrintFinalStat";
	
	delete[] argv;
}

//---------------------------------------------------------
bool GetInputFlags( int &argc, char **&argv, bool &IsNoVecMode, int &GetStartStatMode, 
				    int &SplitInpFileMode )
{
// Get input keys if such exists 
	int i, k;
	stringstream sstream;
	string argv_string,
		   value;

	// default values
	IsNoVecMode      = false;
	GetStartStatMode = 0;
	SplitInpFileMode = 0;

	k = 0;
	// check every input parameters for flag existing
    for ( i = 0; i < argc; i++ ) {
		sstream.str( "" );
		sstream << argv[i];
		argv_string = sstream.str( );
		if ( ( hasPrefix_String( argv_string, "-novecmode",   value ) ) ||
		     ( hasPrefix_String( argv_string, "-no_vec_mode", value ) ) )
			IsNoVecMode = true;
		else if ( ( hasPrefix_String( argv_string, "-GetStartStatMode=", value ) ) ||
		          ( hasPrefix_String( argv_string, "-GetStartStat=",     value ) ) ||
				  ( hasPrefix_String( argv_string, "-getstartstatmode=", value ) ) )
			GetStartStatMode = atoi( value.c_str( ) );
		else if ( ( hasPrefix_String( argv_string, "-SplitInpFile=", value ) ) ||
			      ( hasPrefix_String( argv_string, "-SplitInpFileMode=", value ) )||
		          ( hasPrefix_String( argv_string, "-splitinpfile=", value ) ) )
			SplitInpFileMode = atoi( value.c_str( ) );
		else if ( ( argv_string == "-h"     ) || 
			      ( argv_string == "-help"  ) || 
				  ( argv_string == "--help" ) )
		{ WriteUsage( ); }
		else if ( argv_string[0] == '-' )
            cout << "ERROR! unknown flag " << argv_string;
		else
            argv[k++] = argv[i]; // skip flag arguments
    }
    argc = k;

	return true;
}

int getdir( string dir, vector<string> &files )
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error in opening " << dir << endl;
        return 1;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}