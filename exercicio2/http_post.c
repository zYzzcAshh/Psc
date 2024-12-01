#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>


#define BUFFER_CHUNK (4 * 1024)


struct write_buffer {
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


	memcpy(result->buffer + result->current, ptr, size * nmemb);
	result->current += size * nmemb;


	return size * nmemb;
}


struct read_buffer {
	char *buffer;
	int current, max;
};


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






