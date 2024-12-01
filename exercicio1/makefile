
exercicio1: http_get_json.o http_get.o
	gcc http_get_json.o http_get.o -o exercicio1 -lcurl -ljansson

http_get_json.o: http_get_json.c
	gcc http_get_json.c -lcurl -ljansson -c -g -std=c2x

http_get.o: http_get.c
	gcc http_get.c -lcurl -ljansson -c -g -std=c2x

clean:
	rm *.o exercicio1

