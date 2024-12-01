#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <jansson.h>

char *get_uri(const char *uri);

json_t *http_get_json(const char *url) {
	char *json_string = get_uri(url);
	if (json_string == NULL) {
		fprintf(stderr, "***error: \"%s\" not found\n", url);
		exit(EXIT_FAILURE);
	}
	json_error_t error;
	json_t *root = json_loads(json_string, JSON_DECODE_ANY, &error);
	if (!json_is_object(root)) {
		fprintf(stderr, "***error: on line %d: %s\n", error.line, error.text);
		exit(EXIT_FAILURE);
	}
	return root;
}

int main(int argc, char *argv[]) {
	json_t *root = http_get_json(argv[1]);
	
	if (root) {
        json_dumpf(root, stdout, JSON_INDENT(4)); 
        fprintf(stderr, "Failed to get JSON\n");
    }
	if (root != NULL)
		printf("OK\n");
}
