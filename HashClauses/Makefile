CPP = g++
CPPFLAGS = -O3
	
Hash_clauses: Hash_clauses.cpp Hash_process.o
	${CPP} ${CPPFLAGS} Hash_clauses.cpp Hash_process.o -o Hash_clauses
	
Hash_process.o: Hash_process.cpp
	${CPP} ${CPPFLAGS} Hash_process.cpp -c
	
clean:
	rm -rf *.o
	clear
	
