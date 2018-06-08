server: main.o http.o
	gcc -pthread -o server main.o http.o

main.o: main.c http.h
	gcc -pthread -c main.c

http.o: http.c http.h
	gcc -c http.c

clean:
	rm -f *.o server
