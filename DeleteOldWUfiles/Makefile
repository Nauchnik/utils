CPP = g++
INCDIRS = -I/usr/include/mysql
CPPLIBS = /usr/lib/libmysqlclient.so
CPPFLAGS = -O2 ${INCDIRS}

get_results_info_from_db: main.o
	${CPP} ${CPPFLAGS} ${CPPLIBS} main.o -o delete_old_wu_files

main.o: main.cpp
	${CPP} ${CPPFLAGS} main.cpp -c

clean_obj:
	rm -fr *.o