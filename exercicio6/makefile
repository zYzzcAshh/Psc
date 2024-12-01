wqueue4: wqueue4.o list.o
	gcc wqueue4.o list.o -o wqueue4

wqueue4.o: wqueue4.c
	gcc wqueue4.c -std=c2x -c -g -Wall

list.o: list.c
	gcc list.c -std=c2x -c -g -Wall

clean:
	rm *.o a.out wqueue4
