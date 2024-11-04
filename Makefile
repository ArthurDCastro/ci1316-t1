CFLAGS = -Wall -g -pthread

all: bsearch_function_pa bsearch_function_pb
LIB = chrono.o


bsearch_function_pa: bsearch_function_pa.o $(LIB)
	gcc $(CFLAGS) bsearch_function_pa.o $(LIB) -o bsearch_function_pa

bsearch_function_pa.o : bsearch_function_pa.c 
	gcc $(CFLAGS) -c bsearch_function_pa.c

bsearch_function_pb: bsearch_function_pb.o $(LIB)
	gcc $(CFLAGS) bsearch_function_pb.o $(LIB) -o bsearch_function_pb

bsearch_function_pb.o : bsearch_function_pb.c 
	gcc $(CFLAGS) -c bsearch_function_pb.c

chrono.o: chrono.c chrono.h
	gcc $(CFLAGS) -c chrono.c

clean:
	rm -rf *.o

purge: clean
	rm -rf bsearch_function_pa bsearch_function_pb
