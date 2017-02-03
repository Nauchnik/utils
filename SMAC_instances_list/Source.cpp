#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>

int main()
{
	std::stringstream sstream;
	for (unsigned i = 0; i < 100; i++)
		sstream << "instances/BAR/250/CNF/" << i << "_BAR250_01_8_15.cnf" << std::endl;
	std::ofstream ofile("out.txt");
	ofile << sstream.rdbuf();
	ofile.close();

	return 0;
}
