#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <jansson.h>
#include "help.h"


typedef struct {
    int user_id;
    size_t n_products;
    struct {
        int id;
        size_t quantity;
    } products[];
} Cart;

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

#if 1
int main() {
    Cart *cart = malloc(sizeof(Cart) + 2 * sizeof(cart->products[0])); 
    cart->user_id = 1;
    cart->n_products = 2;
    cart->products[0].id = 144;
    cart->products[0].quantity = 4;
    
    cart->products[1].id = 145;
    cart->products[1].quantity = 2;
    if (cart_put(cart)) {
        printf("Carrinho enviado com sucesso!\n");
    } else {
        fprintf(stderr, "Falha ao enviar o carrinho.\n");
    }
    free(cart); 
    return 0;
}
#endif
