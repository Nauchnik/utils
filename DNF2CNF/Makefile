CPP = g++
CPPFLAGS = -O3 -std=c++0x

dnf2cnf: dnf2cnf.o
	${CPP} ${CPPFLAGS} dnf2cnf.o -o dnf2cnf
	
clean:
	rm -rf *.o
	rm dnf2cnf
	clear