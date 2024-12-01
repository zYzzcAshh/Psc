#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

#include "help.h"

#define MAX_LINE 256

// Funções para comandos
void help();
void list_users(User *users);
void list_products(Product *products, const char *category, int ascending);
void add_to_cart(Cart *cart, Product *products, int product_id, size_t quantity);
void list_cart(Cart *cart);
void finalize_purchase(Cart *cart);

int main()
{
    Users *users = users_get(); 
    Products *products = products_get();   
    Cart *cart = { .user = NULL, .items = NULL };

    char line[MAX_LINE];

    while (true) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL) break;

        char *command = strtok(line, " \n");
        if (command == NULL) continue;

        switch (toupper(*command)) {
            case 'H': // Ajuda
                help();
                break;
            case 'U':  // Assumir utilizador
                char *user_id_str = strtok(NULL, " \n");
                if (user_id_str) {
                    int user_id = atoi(user_id_str);
                    cart.user = find_user(users, user_id);
                    if (cart.user) {
                        printf("Utilizador corrente: %s\n", cart.user->name);
                    } else {
                        printf("Utilizador não encontrado.\n");
                    }
                } else {
                    printf("Comando inválido. Exemplo: U <user_id>\n");
                }
                break;
            case 'P':  // Listar produtos
                char *category = strtok(NULL, " \n");
                char *criteria = strtok(NULL, " \n");
                int ascending = (criteria && strcmp(criteria, "<") == 0);
                list_products(products, category, ascending);
                break;
            case 'C': // Comprar produto
                char *product_id_str = strtok(NULL, " \n");
                char *quantity_str = strtok(NULL, " \n");
                if (product_id_str && quantity_str) {
                    int product_id = atoi(product_id_str);
                    size_t quantity = atoi(quantity_str);
                    add_to_cart(&cart, products, product_id, quantity);
                } else {
                    printf("Comando inválido. Exemplo: C <product_id> <quantity>\n");
                }
                break;
            case 'L': // Listar carrinho
                list_cart(&cart);
                break;
            case 'F': // Finalizar compra
                finalize_purchase(&cart);
                break;
            case 'Q': // Sair
                printf("Saindo do programa...\n");
                exit(0); // Sai do loop
            default:
                printf("Comando desconhecido. Digite 'H' para ajuda.\n");
        };
    };
    free_users(users);
    free_products(products);
    free_cart(&cart);
    return 0;
}