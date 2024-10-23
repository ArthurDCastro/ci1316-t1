CFLAGS = -Wall -g -pthread

all: main

main: src/main.o
	gcc $(CFLAGS) src/main.o -o busca

main.o : src/main.c 
	gcc $(CFLAGS) -c src/main.c

clean:
	rm -rf ./src*.o
	 
purge: clean
	rm -rf busca