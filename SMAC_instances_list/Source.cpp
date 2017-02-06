#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>

int main()
{
	std::stringstream sstream;
	for (unsigned i = 0; i < 100; i++)
		sstream << "instances/BAR/250/GR_CNF/BAR250_" << i << "_gr.cnf" << std::endl;
	std::ofstream ofile("out.txt");
	ofile << sstream.rdbuf();
	ofile.close();
	
	return 0;
}
