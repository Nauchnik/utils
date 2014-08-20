CPP = g++
INC1 = ../boost/
INCDIRS = -I${INC1}
CPPFLAGS = -O3 -std=c++0x ${INCDIRS} -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS

testing_sat_solvers: testing_sat_solvers.o ${CPP} ${CPPFLAGS} testing_sat_solvers.o -o testing_sat_solvers

testing_sat_solvers.o: testing_sat_solvers.cpp
	${CPP} ${CPPFLAGS} testing_sat_solvers.cpp -c
	
clean:
	rm -rf *.o
	rm testing_sat_solvers
	clear