#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include "http_lib.h"

#if 1
int main() {
	char *url = "https://dummyjson.com/products/1";
	json_t *root = http_get_json(url);
	
	if (root) {
        json_dumpf(root, stdout, JSON_INDENT(4)); 
        fprintf(stderr, "Failed to get JSON\n");
    }
	if (root != NULL)
		printf("OK\n");
}
#endif


#if 0
int main() {
    char *url = "https://dummyjson.com/carts/add";
    json_t *data = json_load_file("data.json", 0,NULL); 
    if (!http_post_json(url, data)) {
        fprintf(stderr, "Erro ao realizar POST\n");
        json_decref(data);
    }
    json_decref(data);
    printf("OK\n");
}   
#endif

#if 0
int main() {
    Users *users = users_get();
    if (users) {
        write_users_to_csv(users, "users.csv");
    } else {
        printf("Erro\n");
    }
    return 0;
}
#endif


#if 0
int main() {
    Products *products = products_get();
    if (products) {
        const char *csv_filename = "products.csv";
        write_products_to_csv(products, csv_filename);
        products_free(products);
    } else {
        printf("Erro\n");
    }
    return 0;
}
#endif 


#if 0
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