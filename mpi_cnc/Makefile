CPP = mpicxx
CPPFLAGS = -cxx=g++ -O3 -std=c++11

mpi_cubes: mpi_cubes.o
	${CPP} ${CPPFLAGS} mpi_cubes.o -o mpi_cubes

mpi_cubes.o: mpi_cubes.cpp
	${CPP} ${CPPFLAGS} mpi_cubes.cpp -c

clean:
	rm -rf *.o
	rm mpi_cubes
	clear