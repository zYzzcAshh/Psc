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

struct read_buffer {
	char *buffer;
	int current, max;
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

static size_t read_callback(char *dest, size_t size, size_t nmemb, void *userp)
{
  struct read_buffer *rd = (struct read_buffer *)userp;
  size_t buffer_size = size * nmemb;
 
  if(rd->current < rd->max) {
	/* copy as much as possible from the source to the destination */
	size_t copy_this_much = rd->max - rd->current;
	if (copy_this_much > buffer_size)
		copy_this_much = buffer_size;
	memcpy(dest, rd->buffer + rd->current, copy_this_much);
 
	rd->current -= copy_this_much;
	return copy_this_much; /* we copied this many bytes */
  }
   return 0; /* no more data left to deliver */
}


char *http_post(const char *uri, char *data) {


	curl_global_init(CURL_GLOBAL_DEFAULT);


	CURL *curl = curl_easy_init();


	if (curl != NULL) {


		struct curl_slist *list = NULL;
		list = curl_slist_append(list, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);


		curl_easy_setopt(curl, CURLOPT_URL, uri);


		curl_easy_setopt(curl, CURLOPT_POST, 1L);


		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);


		char *buffer = malloc(BUFFER_CHUNK);
		if (NULL == buffer)
			goto error;


		struct read_buffer read_data = {
			.buffer = buffer,
			.current = 0,
			.max = BUFFER_CHUNK
		};


		read_data.max = strlen(data);
		memcpy(read_data.buffer, data, read_data.max);


		curl_easy_setopt(curl, CURLOPT_READDATA, &read_data);


		curl_easy_setopt(curl, CURLOPT_VERBOSE, 3L);


		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)read_data.max);


		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);


		buffer = malloc(BUFFER_CHUNK);
		if (NULL == buffer)
			goto error;


		struct write_buffer write_result = {
			.buffer = buffer,
			.current = 0,
			.max = BUFFER_CHUNK
		};


		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);


		CURLcode curl_result = curl_easy_perform(curl);
		free(read_data.buffer);


		curl_easy_cleanup(curl);


		if (CURLE_OK != curl_result) {
			fprintf(stderr, "curl told us %d\n", curl_result);
			goto error;
		}


		write_result.buffer[write_result.current] = '\0';


		return write_result.buffer;
	}
error:
	curl_global_cleanup();
	return NULL;
}

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






 