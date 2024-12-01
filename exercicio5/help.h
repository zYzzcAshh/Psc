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



typedef struct Product {
    int id;
    float price;
    char *description;
    char *category;
    struct Product *next; // Para facilitar listas
} Product;

typedef struct User {
    int id;
    char *name;
    struct User *next; // Para listas de utilizadores
} User;

typedef struct CartItem {
    Product *product; // Ponteiro para o produto associado
    size_t quantity;
    struct CartItem *next; // Para listas de itens no carrinho
} CartItem;

typedef struct Cart {
    User *user; // Utilizador associado
    CartItem *items; // Lista de itens no carrinho
} Cart;

struct write_buffer {
	char *buffer;
	int current, max; //current=Quantidade atual de bytes ocupados no buffer
					  //max=Tamanho total alocado do buffer
};

struct read_buffer {
	char *buffer;
	int current, max;
};

// Funções para comandos
void help();
void list_users(User *users);
void list_products(Product *products, const char *category, int ascending);
void add_to_cart(Cart *cart, Product *products, int product_id, size_t quantity);
void list_cart(Cart *cart);
void finalize_purchase(Cart *cart);


json_t *http_get_json(const char *url);

bool http_post_json(const char *url, json_t *data);

bool http_post(const char *url, char *data);

char *get_uri(const char *uri);

typedef struct {
    size_t n_users;
    User *users;
} Users;


Users *users_get();



typedef struct {
    size_t n_products;
    Product *products;
} Products;

Products *products_get()
