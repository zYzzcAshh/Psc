#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <jansson.h>


struct write_buffer {
	char *buffer;
	int current, max; //current=Quantidade atual de bytes ocupados no buffer
					  //max=Tamanho total alocado do buffer
};

struct read_buffer {
	char *buffer;
	int current, max;
};

json_t *http_get_json(const char *url);

bool http_post_json(const char *url, json_t *data);

bool http_post(const char *url, char *data);

char *get_uri(const char *uri);




