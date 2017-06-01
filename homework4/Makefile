all: con4b.o con4a.o

con4b.o:
	gcc -lrt -lpthread -std=c99 con4b.c -o con4b.o

con4a.o:
	gcc -lrt -lpthread -std=c99 con4a.c -o con4a.o

clean:
	rm -rf *.o
