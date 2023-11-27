CPPFLAGS= -g -pthread -I/sw/include/root

main: main.o lockFreeSmartContract.o lockingSmartContract.o
	g++ -O1 main.o lockFreeSmartContract.o lockingSmartContract.o -pthread -ljemalloc

main.o: main.cpp lockFreeSmartContract.cpp lockingSmartContract.cpp lftt.h block.h 
	g++ -O1 -c main.cpp -pthread -ljemalloc

clean:
	-rm *.o

