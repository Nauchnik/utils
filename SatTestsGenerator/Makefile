CPP = g++
INC1 = ../../
INC2 = ../../pdsat/src_common/
INC3 = ../../pdsat/src_mpi/
INC4 = ../../pdsat/src_common/minisat/
INCDIRS = -I${INC1} -I${INC2} -I${INC3} -I${INC4} 
CPPFLAGS = -O3 -std=c++0x ${INCDIRS} -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS

test_generator: test_generator.o mpi_base.o addit_func.o minisat22_wrapper.o Solver.o System.o Options.o
	${CPP} ${CPPFLAGS} test_generator.o mpi_base.o addit_func.o minisat22_wrapper.o Solver.o System.o Options.o -o test_generator

Options.o: ../../pdsat/src_common/minisat/utils/Options.cc
	${CPP} ${CPPFLAGS} ../../pdsat/src_common/minisat/utils/Options.cc -c
	
System.o: ../../pdsat/src_common/minisat/utils/System.cc
	${CPP} ${CPPFLAGS} ../../pdsat/src_common/minisat/utils/System.cc -c

Solver.o: ../../pdsat/src_common/minisat/core/Solver.cc
	${CPP} ${CPPFLAGS} ../../pdsat/src_common/minisat/core/Solver.cc -c
	
minisat22_wrapper.o: ../../pdsat/src_common/minisat22_wrapper.cpp
	${CPP} ${CPPFLAGS} ../../pdsat/src_common/minisat22_wrapper.cpp -c

addit_func.o: ../../pdsat/src_common/addit_func.cpp
	${CPP} ${CPPFLAGS} ../..//pdsat/src_common/addit_func.cpp -c

mpi_base.o: ../../pdsat/src_mpi/mpi_base.cpp
	${CPP} ${CPPFLAGS} ../../pdsat/src_mpi/mpi_base.cpp -c

test_generator.o: test_generator.cpp
	${CPP} ${CPPFLAGS} test_generator.cpp -c
	
clean:
	rm -rf *.o
	rm test_generator
	clear