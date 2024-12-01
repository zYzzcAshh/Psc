#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <jansson.h>

char *http_post(const char *uri, char *data);

bool http_post_json(const char *url, json_t *data) {
    //converter data
    char *json_string = json_dumps(data,4);
    if(!json_string) {
        fprintf(stderr,"Erro na conversao do ficheiro de json para string");
        return false;
    }
    //conversao deu certa
    char *ret = http_post(url,json_string);
    if(ret==NULL){
       fprintf(stderr,"nao conseguei fazer post.Erro!!!!!!!!");
       return false;
    }
    printf("Result: %s\n",ret);
    return true;
}

int main(int argc, char *argv[]) {
    char *url = argv[1];
    json_t *data = json_load_file("data.json", 0,NULL); 
    if (!http_post_json(url, data)) {
        fprintf(stderr, "Erro ao realizar POST\n");
        json_decref(data);
    }
    json_decref(data);
    printf("OK\n");
}   