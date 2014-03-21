CPP = g++
CPPFLAGS = -O2

get_results_info_from_db: main.o
	${CPP} ${CPPFLAGS} main.o -o split_assumption_file

main.o: main.cpp
	${CPP} ${CPPFLAGS} main.cpp -c

clean_obj:
	rm -fr *.o