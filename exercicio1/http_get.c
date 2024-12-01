#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <jansson.h>


#define BUFFER_CHUNK (4 * 1024)

struct write_buffer {
	char *buffer;
	int current, max; //current=Quantidade atual de bytes ocupados no buffer
					  //max=Tamanho total alocado do buffer
};


static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
	struct write_buffer *result = (struct write_buffer *)stream;
    while (result->current + size * nmemb >= result->max - 1) {
		result->buffer = realloc(result->buffer, result->max + BUFFER_CHUNK);
		if (NULL == result->buffer) {
			fprintf(stderr, "Out of memory\n");
			return 0;
		}
		result->max += BUFFER_CHUNK;
	}
	memcpy(result->buffer + result->current, ptr, size * nmemb);//quando a espaco suficiente
	result->current += size * nmemb;
	return size * nmemb; //armanzena dados dinamicamente
}


char *get_uri(const char *uri) {
	curl_global_init(CURL_GLOBAL_DEFAULT); // inicia a libcurl em default
	CURL *curl = curl_easy_init();         // cria um handle curl(configura e executa rquisicoes HTTP)
	if (curl != NULL) { //ve se foi criada com sucesso 
		curl_easy_setopt(curl, CURLOPT_URL, uri); 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);//define write callbaxk  que será chamada toda vez que libcurl receber dados de resposta do servidor
		char *buffer = malloc(BUFFER_CHUNK); //aloca 4k memoria
		if (NULL == buffer) //se alocacao de memoria falhar 
			goto error; //tratamento de dados
		struct write_buffer write_result = {
			.buffer = buffer, //ponteiro dados recebidos
			.current = 0,     //quantidade de bytes usados
			.max = BUFFER_CHUNK //tamanho total alocado para o buffer
		};
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 3L);
		CURLcode curl_result = curl_easy_perform(curl);//Executa a requisição HTTP configurada
		curl_easy_cleanup(curl);// liberta handle CURL   Retorna um código de status (CURLcode),que indica se a operação foi bem-sucedida ou se ocorreu um erro.
		if (CURLE_OK != curl_result) {
			fprintf(stderr, "curl told us %d\n", curl_result);
			goto error;
		}
		write_result.buffer[write_result.current] = '\0';
		return write_result.buffer; //retorna conteudo da resposta http
	}
error:
	curl_global_cleanup();
	return NULL;
}


