
LIBRARY = -lcurl -ljansson
CFLAGS = -g -std=c2x

exercicio3.1: products_get.o help.o
	gcc products_get.o help.o -o main $(LIBRARY) $(CFLAGS)

exercicio3.2: users_get.o help.o
	gcc users_get.o help.o -o main $(LIBRARY) $(CFLAGS)

exercicio3.3: cart_put.o help.o
	gcc cart_put.o help.o -o main $(LIBRARY) $(CFLAGS)

products_get.o: products_get.c
	gcc products_get.c -c $(CFLAGS)

users_get.o: users_get.c
	gcc users_get.c -c $(CFLAGS)

cart_put.o: cart_put.c
	gcc cart_put.c -c $(CFLAGS)	

clean:
	rm *.o main






