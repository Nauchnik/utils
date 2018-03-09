#include "makeSample.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
boost::random::mt19937 gen(static_cast<unsigned int>(time(0)));
using namespace Addit_func;

makeSample::makeSample() :
	launchType(RANDOM_UNSAT_SAMPLE),
	tests_count(0)
{
	gen.seed(static_cast<unsigned>(time(0)));
}

void makeSample::readInput(int argc, char **argv)
{
	cnf_file_name = argv[1];
	cout << "cnf_file_name " << cnf_file_name << endl;
	cut_cnf_file_name = cnf_file_name;
	while ((cut_cnf_file_name[0] == '.') || (cut_cnf_file_name[0] == '/'))
		cut_cnf_file_name = cut_cnf_file_name.substr(1, cut_cnf_file_name.length() - 1);
	cout << "cut_cnf_file_name " << cut_cnf_file_name << endl;

	decomp_set_file_name = argv[2];
	cout << "decomp_set_file_name " << decomp_set_file_name << endl;

	string third_param_str = argv[3];
	
	if ( third_param_str == "-input_output_assumptions" ) {
		launchType = INPUT_OUTPUT_ASSUMPTIONS_SAMPLE;
		cout << "launchType " << launchType << endl;
		input_output_folder_name = argv[4];
		cout << "input_output_folder_name " << input_output_folder_name << endl;
		return;
	}
	
	tests_count = atoi(third_param_str.c_str());
	cout << "tests_count " << tests_count << endl;

	string str;
	if (argc > 4) {
		str = argv[4];
		if (str.find("sat") != string::npos)
			launchType = RANDOM_SAT_SAMPLE;
		else if (str.find("assumptions") != string::npos) {
			if (decomp_set_file_name != "no")
				launchType = ASSUMPTIONS_SAMPLE;
			else
				launchType = INPUT_ASSUMPTIONS_SAMPLE;
			assumptions_file_name = argv[5];
		}
	}
		
	cout << "launchType " << launchType << endl;
	cout << "assumptions_file_name " << assumptions_file_name << endl;
}

void makeSample::init()
{
	cnf_file.open(cnf_file_name.c_str());
	if (!cnf_file.is_open()) {
		cerr << "Error. !cnf_file.is_open()" << endl;
		exit(1);
	}

	test_cnf_files.resize(tests_count);

	unsigned found = cut_cnf_file_name.find(".");
	cnf_name_common_part = (found != string::npos) ? cut_cnf_file_name.substr(0, found) : cut_cnf_file_name;
	cout << "cnf_name_common_part " << cnf_name_common_part << endl;

	unsigned comment_str_count = 0, main_str_count = 0;
	string str;
	while (getline(cnf_file, str)) {
		str.erase(remove(str.begin(), str.end(), '\r'), str.end());
		if (str[0] == 'c') {
			comment_cnf_sstream << str << endl;
			comment_str_count++;
		}
		else if (str[0] != 'p') {
			if (main_str_count)
				main_cnf_sstream << endl;
			main_cnf_sstream << str;
			main_str_count++;
		}
	}
	cout << "main_str_count " << main_str_count << endl;

	bool isDecompSet = false;
	if (decomp_set_file_name != "no") {
		decomp_set_file.open(decomp_set_file_name.c_str());
		isDecompSet = true;
		if (!decomp_set_file.is_open()) {
			cerr << "Error. !cnf_file.is_open()" << endl;
			exit(1);
		}
	}
	int from, to;
	if (isDecompSet) {
		unsigned val = 0;
		cout << "reading decomp_set" << endl;
		getline(decomp_set_file, str);
		decomp_set_file.close();
		decomp_set_file.clear();
		val = str.find('-');
		cout << "val " << val << endl;
		if ( val < 1000000 ) {
			cout << "reading interval" << endl;
			string val_from_str, val_to_str;
			val_from_str = str.substr( 0, val );
			val_to_str = str.substr(val + 1, str.size() - val_from_str.size());
			istringstream(val_from_str) >> from;
			istringstream(val_to_str) >> to;
			for (int i = from; i <= to; i++ )
				decomp_set.push_back(i);
		}
		else {
			cout << "reading separate variables" << endl;
			decomp_set_file.open(decomp_set_file_name.c_str());
			while (decomp_set_file >> val) {
				cout << val << " ";
				decomp_set.push_back(val);
			}
			decomp_set_file.close();
		}
	}
	cout << endl;
	cout << "decomp_set.size() " << decomp_set.size() << endl;

	mpi_b.isMakeSatSampleAnyWay = true;
	mpi_b.input_cnf_name = cut_cnf_file_name;
	mpi_b.ReadIntCNF();
	cout << "mpi_b.var_count " << mpi_b.var_count << endl;
	cout << "mpi_b.clause_count " << mpi_b.clause_count << endl;
	
	for (unsigned i = 0; i < mpi_b.output_len; i++)
		output_variables.push_back(mpi_b.var_count - mpi_b.output_len + i + 1);
	for (unsigned i = 0; i < mpi_b.core_len; i++)
		input_variables.push_back(i + 1);

	vector<unsigned>::iterator it;
	for (unsigned i = 0; i < decomp_set.size(); i++) {
		it = find(input_variables.begin(), input_variables.end(), decomp_set[i]);
		if (it != input_variables.end())
			decomp_set_indexes.push_back(it - input_variables.begin());
	}

	unsigned new_clause_count;
	if ((launchType == RANDOM_SAT_SAMPLE) || (launchType == INPUT_OUTPUT_ASSUMPTIONS_SAMPLE) || 
		(launchType == INPUT_ASSUMPTIONS_SAMPLE))
		new_clause_count = mpi_b.clause_count + decomp_set.size() + output_variables.size();
	else
		new_clause_count = mpi_b.clause_count + decomp_set.size();

	head_cnf_sstream << "p cnf " << mpi_b.var_count << " " << new_clause_count << endl;

	cnf_file.close();
}

void makeSample::makeRandomUnsatSample()
{
	stringstream current_name_sstream;
	for (unsigned i = 0; i < test_cnf_files.size(); i++) {
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new ofstream(current_name_sstream.str().c_str());
		(*test_cnf_files[i]) << comment_cnf_sstream.str(); // write comments of cnf file
		(*test_cnf_files[i]) << head_cnf_sstream.str(); // write head of cnf file
														// write oneliteral clauses
		for (unsigned j = 0; j < decomp_set.size(); j++) {
			if (bool_rand(gen))
				(*test_cnf_files[i]) << "-";
			(*test_cnf_files[i]) << decomp_set[j] << " 0" << endl;
		}

		(*test_cnf_files[i]) << main_cnf_sstream.str(); // write clauses of main cnf
		(*test_cnf_files[i]).close();
		delete test_cnf_files[i];
		current_name_sstream.clear(); current_name_sstream.str("");
	}
	cout << tests_count << " tests were created" << endl;
}

void makeSample::makeRandomSatSample()
{
	cout << "Start makeRandomSatSample()" << endl;

	vector< vector<bool> > plain_text_vec_vec;
	mpi_b.cnf_in_set_count = tests_count;
	mpi_b.MakeSatSample(state_vec_vec, stream_vec_vec, plain_text_vec_vec, 0);

	unsigned cur_var_ind;
	stringstream oneliteral_sstream, current_name_sstream;
	for (unsigned i = 0; i < tests_count; i++) {
		for (vector<unsigned>::iterator it = decomp_set.begin(); it != decomp_set.end(); it++) {
			cur_var_ind = (*it) - 1;
			if (!(state_vec_vec[i][cur_var_ind]))
				oneliteral_sstream << "-";
			oneliteral_sstream << cur_var_ind + 1 << " 0" << endl;
		}
		unsigned cur_stream_index = 0;
		for (vector<bool>::iterator it = stream_vec_vec[i].begin(); it != stream_vec_vec[i].end(); it++) {
			cur_var_ind = (mpi_b.var_count - mpi_b.output_len) + cur_stream_index;
			if (!(*it))
				oneliteral_sstream << "-";
			oneliteral_sstream << cur_var_ind + 1 << " 0" << endl;
			cur_stream_index++;
		}
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new ofstream(current_name_sstream.str().c_str());
		(*test_cnf_files[i]) << comment_cnf_sstream.str(); // write comments of cnf file
		(*test_cnf_files[i]) << head_cnf_sstream.str();    // write head of cnf file
		(*test_cnf_files[i]) << oneliteral_sstream.str();  // write oneliteral clauses
		(*test_cnf_files[i]) << main_cnf_sstream.str();    // write clauses of main cnf
		(*test_cnf_files[i]).close();
		delete test_cnf_files[i];
		current_name_sstream.clear(); current_name_sstream.str("");
		oneliteral_sstream.str(""); oneliteral_sstream.clear();
	}
}

void makeSample::makeSampleFromInputs()
{
	cout << "Start makeSampleFromInputs()" << endl;

	ifstream assumptions_file(assumptions_file_name);
	if (!assumptions_file.is_open()) {
		cerr << "assumptions_file " << assumptions_file_name << " open fails " << endl;
		exit(1);
	}
	string str;
	vector<string> var_values_vec;
	string tmp_str;
	bool isMatchString;
	vector<bool> tmp_state_vec;
	while (getline(assumptions_file, str)) { // read values of variables
		for (unsigned i = 0; i < str.size(); i++) {
			if (str[i] == '1')
				tmp_state_vec.push_back(true);
			else if (str[i] == '0')
				tmp_state_vec.push_back(false);
		}
		state_vec_vec.push_back(tmp_state_vec);
		tmp_state_vec.clear();
	}
	cout << "state_vec_vec.size() " << state_vec_vec.size() << endl;

	vector< vector<bool> > plain_text_vec_vec;
	mpi_b.cnf_in_set_count = tests_count;
	mpi_b.MakeSatSample(state_vec_vec, stream_vec_vec, plain_text_vec_vec, 0);
	
	cout << "stream_vec_vec.size() " << stream_vec_vec.size() << endl;

	unsigned cur_var_ind;
	tests_count = stream_vec_vec.size();
	stringstream oneliteral_sstream, current_name_sstream;
	for (unsigned i = 0; i < tests_count; i++) {
		unsigned cur_stream_index = 0;
		for (vector<bool>::iterator it = stream_vec_vec[i].begin(); it != stream_vec_vec[i].end(); it++) {
			cur_var_ind = (mpi_b.var_count - mpi_b.output_len) + cur_stream_index;
			if (!(*it))
				oneliteral_sstream << "-";
			oneliteral_sstream << cur_var_ind + 1 << " 0" << endl;
			cur_stream_index++;
		}
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new ofstream(current_name_sstream.str().c_str());
		(*test_cnf_files[i]) << comment_cnf_sstream.str(); // write comments of cnf file
		(*test_cnf_files[i]) << head_cnf_sstream.str();    // write head of cnf file
		(*test_cnf_files[i]) << oneliteral_sstream.str();  // write oneliteral clauses
		(*test_cnf_files[i]) << main_cnf_sstream.str();    // write clauses of main cnf
		(*test_cnf_files[i]).close();
		delete test_cnf_files[i];
		current_name_sstream.clear(); current_name_sstream.str("");
		oneliteral_sstream.str(""); oneliteral_sstream.clear();
	}
}

void makeSample::makeSampleFromAssumptions()
{
	ifstream assumptions_file(assumptions_file_name);
	if (!assumptions_file.is_open()) {
		cerr << "assumptions_file " << assumptions_file_name << " open fails " << endl;
		exit(1);
	}
	string str;
	vector<string> var_values_vec;
	stringstream sstream;
	string tmp_str;
	bool isMatchString;
	while (getline(assumptions_file, str)) { // read values of variables
		sstream << str;
		while (sstream >> tmp_str) {
			if (tmp_str.size() < 5)
				continue;
			isMatchString = true;
			for (auto &x : tmp_str)
				if ((x != '1') && (x != '0')) {
					isMatchString = false;
					break;
				}
			if (isMatchString)
				break;
		}
		var_values_vec.push_back(tmp_str);
		sstream.clear(); sstream.str("");
	}

	assumptions_file.close();
	tests_count = var_values_vec.size();
	test_cnf_files.resize(tests_count);
	stringstream current_name_sstream;
	for (unsigned i = 0; i < test_cnf_files.size(); i++) {
		current_name_sstream << cnf_name_common_part << "_" << i << ".cnf";
		test_cnf_files[i] = new ofstream(current_name_sstream.str().c_str());
		(*test_cnf_files[i]) << comment_cnf_sstream.str(); // write comments of cnf file
		(*test_cnf_files[i]) << head_cnf_sstream.str(); // write head of cnf file
														// write oneliteral clauses
		for (unsigned j = 0; j < decomp_set.size(); j++) // write corresponding oneliteral clauses
			(*test_cnf_files[i]) << (var_values_vec[i][j] == '1' ? "" : "-") << decomp_set[j] << " 0" << endl;
		(*test_cnf_files[i]) << main_cnf_sstream.str(); // write clauses of main cnf
		(*test_cnf_files[i]).close();
		delete test_cnf_files[i];
		current_name_sstream.clear(); current_name_sstream.str("");
	}

	cout << tests_count << " tests were created" << endl;
}

void makeSample::makeSampleFromInputOutputAssumptions()
{
	vector<string> input_output_files_names = vector<string>();

	Addit_func::getdir(input_output_folder_name, input_output_files_names);
	
	cout << "input_output_files_names.size() " << input_output_files_names.size() << endl;
	
	string cur_file_name;
	fstream cur_file;
	string input_variables_values_str, output_variables_values_str, start_str;
	stringstream oneliteral_sstream;
	string cnf_folder_name = input_output_folder_name + "_cnfs";

	for ( auto &x : input_output_files_names ) {
		cur_file_name = "./" + input_output_folder_name + "/" + x;
		if (cur_file_name.find(".cnf") != string::npos)
			continue;
		cur_file.open(cur_file_name.c_str(), ios_base::in);
		if (!cur_file.is_open()) {
			cerr << "couldn't open " << cur_file_name << endl;
			exit(1);
		}
		getline(cur_file, input_variables_values_str);
		start_str = "Input: ";
		input_variables_values_str = input_variables_values_str.substr(start_str.size(), input_variables_values_str.size() - start_str.size());
		getline(cur_file, output_variables_values_str);
		start_str = "Output: ";
		output_variables_values_str = output_variables_values_str.substr(start_str.size(), output_variables_values_str.size() - start_str.size());
		if ( (mpi_b.output_len>0) && (mpi_b.output_len < output_variables_values_str.size()) )
			output_variables_values_str.resize(mpi_b.output_len);
		cur_file.clear();
		cur_file.close();
		// make CNF file with additional clauses
		cur_file_name = "./" + cnf_folder_name + "/" + x + ".cnf";
		cur_file.open(cur_file_name.c_str(), ios_base::out);
		if (!cur_file.is_open()) {
			cerr << "couldn't open " << cur_file_name << endl;
			exit(1);
		}
		for (unsigned i = 0; i < decomp_set_indexes.size(); i++) {
			if (input_variables_values_str[decomp_set_indexes[i]] == '0')
				oneliteral_sstream << "-";
			oneliteral_sstream << decomp_set[i] << " 0" << endl;
		}
		//for (cur_stream_index = 0; cur_stream_index < mpi_b.keystream_len; cur_stream_index++) {
		for (unsigned i = 0; i < output_variables.size(); i++) {
			if (output_variables_values_str[i] == '0')
				oneliteral_sstream << "-";
			oneliteral_sstream << output_variables[i] << " 0" << endl;
		}
		cur_file << comment_cnf_sstream.str(); // write comments of cnf file
		cur_file << head_cnf_sstream.str();    // write head of cnf file
		cur_file << oneliteral_sstream.str();  // write oneliteral clauses
		cur_file << main_cnf_sstream.str();    // write clauses of main cnf
		cur_file.clear();
		cur_file.close();
		oneliteral_sstream.str(""); oneliteral_sstream.clear();
	}
}