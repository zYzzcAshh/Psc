main: main.o help.o
	gcc main.o help.o -o main

main.o: main.c
	gcc main.c -std=c2x -c -g -Wall

help.o: help.c
	gcc help.c -std=c2x -c -g -Wall

clean:
	rm *.o a.out main
