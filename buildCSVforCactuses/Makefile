CPP = g++
CPPFLAGS = -O3 -std=c++0x -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS

build_csv: build_csv.o 
	${CPP} ${CPPFLAGS} build_csv.o -o build_csv

build_csv.o: build_csv.cpp
	${CPP} ${CPPFLAGS} build_csv.cpp -c
	
clean:
	rm -rf *.o
	rm build_csv
	clear