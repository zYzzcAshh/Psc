#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <jansson.h>


#include "help.h" // Inclui as funções anteriores

#define MAX_LINE 256

typedef struct {
    int id;
    const char *name;
} User;

typedef struct {
    size_t n_users;
    User *users;
} Users;

void help() {
    printf("Comandos disponíveis:\n");
    printf("H                  - Mostrar ajuda\n");
    printf("U <user_id>        - Assumir utilizador\n");
    printf("P <categoria> <c|d> - Listar produtos por categoria (crescente ou decrescente)\n");
    printf("C <produto> <quantidade> - Comprar produto\n");
    printf("L                  - Listar produtos no carrinho\n");
    printf("F                  - Finalizar compra\n");
    printf("Q                  - Sair\n");
}

void list_users(User *users) {
    printf("Lista de utilizadores:\n");
    for (User *u = users; u != NULL; u = u->next) {
        printf("ID: %d, Nome: %s\n", u->id, u->name);
    }
}

void list_products(Product *products, const char *category, int ascending) {
    Product *filtered = NULL;
    Product *current = products;

    // Filtrar por categoria
    while (current) {
        if (!category || strcmp(current->category, category) == 0) {
            Product *new_product = malloc(sizeof(Product));
            *new_product = *current; // Copia o produto
            new_product->next = filtered;
            filtered = new_product;
        }
        current = current->next;
    }

    // Ordenar a lista filtrada por preço
    for (Product *p1 = filtered; p1 != NULL; p1 = p1->next) {
        for (Product *p2 = p1->next; p2 != NULL; p2 = p2->next) {
            if ((ascending && p1->price > p2->price) ||
                (!ascending && p1->price < p2->price)) {
                Product temp = *p1;
                *p1 = *p2;
                *p2 = temp;
            }
        }
    }

    // Listar os produtos
    printf("Lista de produtos:\n");
    for (Product *p = filtered; p != NULL; p = p->next) {
        printf("ID: %d, Descrição: %s, Categoria: %s, Preço: %.2f\n",
               p->id, p->description, p->category, p->price);
    }

    // Liberar a lista filtrada
    while (filtered) {
        Product *temp = filtered;
        filtered = filtered->next;
        free(temp);
    }
}

void add_to_cart(Cart *cart, Product *products, int product_id, size_t quantity) {
    // Procurar o produto na lista
    Product *product = NULL;
    for (Product *p = products; p != NULL; p = p->next) {
        if (p->id == product_id) {
            product = p;
            break;
        }
    }

    if (!product) {
        printf("Produto não encontrado.\n");
        return;
    }

    // Verificar se já existe no carrinho
    CartItem *current = cart->items;
    while (current) {
        if (current->product->id == product_id) {
            current->quantity += quantity;
            printf("Quantidade atualizada no carrinho: %zu\n", current->quantity);
            return;
        }
        current = current->next;
    }

    // Adicionar novo item ao carrinho
    CartItem *new_item = malloc(sizeof(CartItem));
    new_item->product = product;
    new_item->quantity = quantity;
    new_item->next = cart->items;
    cart->items = new_item;

    printf("Produto %s adicionado ao carrinho.\n", product->description);
}

void list_cart(Cart *cart) {
    if (!cart->items) {
        printf("Carrinho vazio.\n");
        return;
    }

    printf("Carrinho de compras:\n");
    float total = 0.0;
    for (CartItem *item = cart->items; item != NULL; item = item->next) {
        float subtotal = item->product->price * item->quantity;
        total += subtotal;
        printf("ID: %d, Descrição: %s, Preço: %.2f, Quantidade: %zu, Subtotal: %.2f\n",
               item->product->id, item->product->description, item->product->price,
               item->quantity, subtotal);
    }
    printf("Total: %.2f\n", total);
}

void finalize_purchase(Cart *cart) {
    if (!cart->items) {
        printf("Carrinho vazio.\n");
        return;
    }

    // Simular envio para API (ou chamar `cart_put`)
    printf("Finalizando compra...\n");

    // Mostrar resumo da compra
    list_cart(cart);

    // Limpar itens do carrinho
    while (cart->items) {
        CartItem *temp = cart->items;
        cart->items = cart->items->next;
        free(temp);
    }

    printf("Compra finalizada com sucesso. Carrinho esvaziado.\n");
}



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



 typedef struct {
    int id;
    float price;
    const char *description;
    const char *category;
} Product;

typedef struct {
    size_t n_products;
    Product *products;
} Products;

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



