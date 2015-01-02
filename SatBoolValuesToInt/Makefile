CPP = g++
INC1 = ../../
INCDIRS = -I${INC1} 
CPPFLAGS = -O3 -std=c++0x ${INCDIRS} -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS

index: index.o
	${CPP} ${CPPFLAGS} index.o -o index

index.o: index.cpp
	${CPP} ${CPPFLAGS} index.cpp -c

clean:
	rm -rf *.o
	rm test
	clear