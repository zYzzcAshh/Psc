
exercicio2: http_post_json.o http_post.o
	gcc http_post_json.o http_post.o -o exercicio2 -lcurl -ljansson

http_post_json.o: http_post_json.c
	gcc http_post_json.c -lcurl -ljansson -c -g -std=c2x

http_post.o: http_post.c
	gcc http_post.c -lcurl -ljansson -c -g -std=c2x

clean:
	rm -f *.o exercicio2

