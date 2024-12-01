#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <jansson.h>
#include "help.h"
/*
    Na função http_get.c foi colocado em comentario o troço de
    codigo responsavel por printar a response do curl.
*/

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

#if 1
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