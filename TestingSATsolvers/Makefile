CPP = mpicxx
INC1 = ../../
INC2 = ../../pdsat/src_common/
INCDIRS = -I${INC1} -I${INC2}
CPPFLAGS = -O3 -std=c++0x ${INCDIRS} -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -D _MPI

testing_sat_solvers: addit_func.o testing_sat_solvers.o
	${CPP} ${CPPFLAGS} addit_func.o testing_sat_solvers.o -o testing_sat_solvers

testing_sat_solvers.o: testing_sat_solvers.cpp
	${CPP} ${CPPFLAGS} testing_sat_solvers.cpp -c

addit_func.o: ../../pdsat/src_common/addit_func.cpp
	${CPP} ${CPPFLAGS} ../../pdsat/src_common/addit_func.cpp -c

clean:
	rm -rf *.o
	rm testing_sat_solvers
	clear
