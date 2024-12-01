#ifndef HTTP_LIB_H
#define HTTP_LIB_H

#include <jansson.h>
#include <stdbool.h>

typedef struct {
    int id;
    const char *name;
} User;

typedef struct {
    size_t n_users;
    User *users;
} Users;

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

typedef struct {
    int user_id;
    size_t n_products;
    struct {
        int id;
        size_t quantity;
    } products[];
} Cart;

json_t *http_get_json(const char *url);
bool http_post_json(const char *url, json_t *data);
char *http_post(const char *uri, char *data);
char *get_uri(const char *uri);

void write_users_to_csv(const Users *users, const char *filename);
void users_free(Users *users);
Users *users_get();

Products *products_get();
void products_free(Products *products);
void write_products_to_csv(const Products *products, const char *filename);

bool cart_put(const Cart *cart);
json_t *cart_to_json(const Cart *cart);

#endif
