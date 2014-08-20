CPP = g++
CPPFLAGS = -O3 -std=c++0x -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS

fix_cnf: fix_cnf.o 
	${CPP} ${CPPFLAGS} fix_cnf.o -o fix_cnf

fix_cnf.o: fix_cnf.cpp
	${CPP} ${CPPFLAGS} fix_cnf.cpp -c
	
clean:
	rm -rf *.o
	rm fix_cnf
	clear