#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

#define clause vector<int>

bool compareBySize(const clause &a, const clause &b)
{
	return a.size() < b.size();
}

int main() {
	string drup_name = "proof_geffe.out";
	ifstream ifile(drup_name);
	vector<clause> good_clauses;
	vector<clause> bad_clauses;
	vector<clause> diff_clauses;
	string str;

	while (getline(ifile,str)) {
		bool isBad = false;
		if (str.substr(0, 2) == "d ") {
			isBad = true;
			str = str.substr(2, str.size() - 2);
		}

		stringstream sstream;
		sstream << str;
		clause cla;
		int val;
		while (sstream >> val) {
			if (val)
				cla.push_back(val);
		}

		sort(cla.begin(), cla.end());

		if (isBad)
			bad_clauses.push_back(cla);
		else
			good_clauses.push_back(cla);
	}

	ifile.close();
	cout << "good_clauses size " << good_clauses.size() << endl;
	cout << "bad_clauses size " << bad_clauses.size() << endl;
	
	sort(good_clauses.begin(), good_clauses.end());
	sort(bad_clauses.begin(), bad_clauses.end());
	unsigned diff_iterations = 0;
	for (;;) {
		set_difference(good_clauses.begin(), good_clauses.end(), bad_clauses.begin(), bad_clauses.end(),
			inserter(diff_clauses, diff_clauses.begin()));
		diff_iterations++;
		cout << "diff_iterations " << diff_iterations << endl;
		if (good_clauses.size() == diff_clauses.size())
			break;
		else {
			cout << "new iteration required " << endl;
			cout << "good_clauses size " << good_clauses.size() << endl;
			cout << "diff_clauses size " << diff_clauses.size() << endl;
			good_clauses = diff_clauses;
			diff_clauses.clear();
		}
	}
	sort(good_clauses.begin(), good_clauses.end(), compareBySize);
	sort(bad_clauses.begin(), bad_clauses.end(), compareBySize);
	cout << "final good_clauses size " << good_clauses.size() << endl;
	ofstream good_clauses_file("good_clauses");
	ofstream bad_clauses_file("bad_clauses");
	for (auto x : good_clauses) {
		for (auto y : x)
			good_clauses_file << y << " ";
		good_clauses_file << "\n";
	}
	for (auto x : bad_clauses) {
		for (auto y : x)
			bad_clauses_file << y << " ";
		bad_clauses_file << "\n";
	}
	good_clauses_file.close();
	bad_clauses_file.close();
	
	return 0;
}