CPP = g++
INC1 = ../../
INC2 = ../../pdsat/src_common/
INCDIRS = -I${INC1} -I${INC2}
CPPFLAGS = -O3 -std=c++0x ${INCDIRS} -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS

read_ursa_solving_time: addit_func.o read_ursa_solving_time.o
	${CPP} ${CPPFLAGS} addit_func.o read_ursa_solving_time.o -o read_ursa_solving_time

read_ursa_solving_time.o: read_ursa_solving_time.cpp
	${CPP} ${CPPFLAGS} read_ursa_solving_time.cpp -c

addit_func.o: ../../pdsat/src_common/addit_func.cpp
	${CPP} ${CPPFLAGS} ../../pdsat/src_common/addit_func.cpp -c

clean:
	rm -rf *.o
	rm read_ursa_solving_time
	clear