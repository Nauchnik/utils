CPP = mpicxx
CPPFLAGS = -O3 -std=c++0x -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -D _MPI

mpi_cubes: mpi_cubes.o
	${CPP} ${CPPFLAGS} mpi_cubes.o -o mpi_cubes

mpi_cubes.o: mpi_cubes.cpp
	${CPP} ${CPPFLAGS} mpi_cubes.cpp -c

clean:
	rm -rf *.o
	rm mpi_cubes
	clear