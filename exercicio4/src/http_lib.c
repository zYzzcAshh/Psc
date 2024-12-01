#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "http_lib.h"

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
    memcpy(result->buffer + result->current, ptr, size * nmemb);
    result->current += size * nmemb;
    return size * nmemb;
}

static size_t read_callback(char *dest, size_t size, size_t nmemb, void *userp)
{
    struct read_buffer *rd = (struct read_buffer *)userp;
    size_t buffer_size = size * nmemb;
 
    if(rd->current < rd->max) {
        size_t copy_this_much = rd->max - rd->current;
        if (copy_this_much > buffer_size)
            copy_this_much = buffer_size;
        memcpy(dest, rd->buffer + rd->current, copy_this_much);
        rd->current -= copy_this_much;
        return copy_this_much;
    }
    return 0;
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
    char *json_string = json_dumps(data, 4);
    if (!json_string) {
        fprintf(stderr, "Erro na conversao do ficheiro de json para string\n");
        return false;
    }
    char *ret = http_post(url, json_string);
    free(json_string);
    if (ret == NULL) {
        fprintf(stderr, "Falha ao enviar POST\n");
        return false;
    }
    printf("Resultado: %s\n", ret);
    free(ret); // Free memory after use
    return true;
}

char *get_uri(const char *uri) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();
    if (curl != NULL) {
        curl_easy_setopt(curl, CURLOPT_URL, uri);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        char *buffer = malloc(BUFFER_CHUNK);
        if (NULL == buffer)
            goto error;

        struct write_buffer write_result = {
            .buffer = buffer,
            .current = 0,
            .max = BUFFER_CHUNK
        };
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 3L);

        CURLcode curl_result = curl_easy_perform(curl);
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

json_t *http_get_json(const char *url) {
    char *json_string = get_uri(url);
    if (json_string == NULL) {
        fprintf(stderr, "***error: \"%s\" not found\n", url);
        exit(EXIT_FAILURE);
    }
    json_error_t error;
    json_t *root = json_loads(json_string, JSON_DECODE_ANY, &error);
    free(json_string);
    if (!json_is_object(root)) {
        fprintf(stderr, "***error: on line %d: %s\n", error.line, error.text);
        exit(EXIT_FAILURE);
    }
    return root;
}
Users *users_get() {
    json_t *root = http_get_json("https://dummyjson.com/users");
    if (!root) {
        return NULL;
    }

    json_t *users_array = json_object_get(root, "users");
    if (!json_is_array(users_array)) {
        json_decref(root);
        return NULL;
    }

    Users *users = malloc(sizeof(Users));
    if (!users) {
        json_decref(root);
        return NULL;
    }

    users->n_users = json_array_size(users_array);
    users->users = malloc(sizeof(User) * users->n_users);
    if (!users->users) {
        free(users);
        json_decref(root);
        return NULL;
    }

    for (size_t i = 0; i < users->n_users; i++) {
        json_t *user = json_array_get(users_array, i);
        
        json_t *id = json_object_get(user, "id");
        json_t *name = json_object_get(user, "username");

        users->users[i].id = json_integer_value(id);
        users->users[i].name = strdup(json_string_value(name));
    }

    json_decref(root);
    return users;
}

void users_free(Users *users) {
    if (!users) return;
    
    for (size_t i = 0; i < users->n_users; i++) {
        free((void*)users->users[i].name);
    }
    free(users->users);
    free(users);
}

void write_users_to_csv(const Users *users, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erro ao abrir o ficheiro para escrita");
        return;
    }
    fprintf(file, "%-5s %-20s\n", "ID", "Name");
    for (size_t i = 0; i < users->n_users; i++) {
        fprintf(file, "%-5d %-20s\n", 
                users->users[i].id, 
                users->users[i].name ? users->users[i].name : "Desconhecido");
    }
    fclose(file);
    printf("Usuários exportados para '%s'.\n", filename);
}



Products *products_get() {
    json_t *root = http_get_json("https://dummyjson.com/products");
    if (!root) {
        return NULL;
    }

    json_t *products_array = json_object_get(root, "products");
    if (!json_is_array(products_array)) {
        json_decref(root);
        return NULL;
    }

    Products *products = malloc(sizeof(Products));
    if (!products) {
        json_decref(root);
        return NULL;
    }

    products->n_products = json_array_size(products_array);
    products->products = malloc(sizeof(Product) * products->n_products);
    if (!products->products) {
        free(products);
        json_decref(root);
        return NULL;
    }

    for (size_t i = 0; i < products->n_products; i++) {
        json_t *product = json_array_get(products_array, i);
        
        json_t *id = json_object_get(product, "id");
        json_t *price = json_object_get(product, "price");
        json_t *description = json_object_get(product, "description");
        json_t *category = json_object_get(product, "category");

        products->products[i].id = json_integer_value(id);
        products->products[i].price = (float)json_number_value(price);
        products->products[i].description = strdup(json_string_value(description));
        products->products[i].category = strdup(json_string_value(category));
    }

    json_decref(root);
    return products;
}

void products_free(Products *products) {
    if (!products) return;
    for (size_t i = 0; i < products->n_products; i++) {
        free((void*)products->products[i].description);
        free((void*)products->products[i].category);
    }
    free(products->products);
    free(products);
}

void write_products_to_csv(const Products *products, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erro ao abrir o ficheiro para escrita");
        return;
    }
    fprintf(file, "%-5s %-10s %-15s %-100s\n", "ID", "Price", "Category", "Description");
    for (size_t i = 0; i < products->n_products; i++) {
        fprintf(file, "%-5d %-10.2f %-15s %-100s\n",
                products->products[i].id,
                products->products[i].price,
                products->products[i].category,
                products->products[i].description);
    }
    fclose(file);
    printf("Produtos exportados para '%s'.\n", filename);
}

json_t *cart_to_json(const Cart *cart) {
    json_t *root = json_object(); 
    json_object_set_new(root, "userId", json_integer(cart->user_id)); 

    json_t *products_array = json_array(); 
    for (size_t i = 0; i < cart->n_products; i++) {
        json_t *product = json_object(); 
        json_object_set_new(product, "id", json_integer(cart->products[i].id));
        json_object_set_new(product, "quantity", json_integer(cart->products[i].quantity));
        json_array_append_new(products_array, product);
    }

    json_object_set_new(root, "products", products_array);
    return root; 
}

bool cart_put(const Cart *cart) {
    if (!cart) {
        fprintf(stderr, "Erro: carrinho de compras nulo.\n");
        return false;
    }
    const char *url = "https://dummyjson.com/carts/add";
    json_t *cart_json = cart_to_json(cart);
    if (!cart_json) {
        fprintf(stderr, "Erro ao converter o carrinho de compras para JSON.\n");
        return false;
    }
    bool result = http_post_json(url, cart_json);
    json_decref(cart_json); 
    return result;
}

