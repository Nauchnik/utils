CPP = g++
INC1 = ../../DiagonalLatinSquaresGenerator/
INCDIRS = -I${INC1}
CPPFLAGS = -O3 -std=c++0x ${INCDIRS}

ConstructPseudotripleCNFs: Source.o odls_sequential.o
	${CPP} ${CPPFLAGS} Source.o odls_sequential.o -o ConstructPseudotripleCNFs

odls_sequential.o: ../../DiagonalLatinSquaresGenerator/odls_sequential.cpp
	${CPP} ${CPPFLAGS} ../../DiagonalLatinSquaresGenerator/odls_sequential.cpp -c

Source.o: ./Source.cpp
	${CPP} ${CPPFLAGS} ./Source.cpp -c

clean:
	rm -rf *.o
	rm ConstructPseudotripleCNFs
	clear