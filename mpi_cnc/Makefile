CPP = g++
CPPFLAGS = -O3 -std=c++20 -fopenmp

parallel_cubes: parallel_cubes.o
	${CPP} ${CPPFLAGS} parallel_cubes.o -o parallel_cubes

parallel_cubes.o: parallel_cubes.cpp
	${CPP} ${CPPFLAGS} parallel_cubes.cpp -c

clean:
	rm -rf *.o
	rm parallel_cubes
	clear
