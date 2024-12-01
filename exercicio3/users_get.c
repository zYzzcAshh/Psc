#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <jansson.h>

/*
    Na função http_get.c foi colocado em comentario o troço de
    codigo responsavel por printar a response do curl.
*/

typedef struct {
    int id;
    const char *name;
} User;

typedef struct {
    size_t n_users;
    User *users;
} Users;

json_t *http_get_json(const char *url);

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

int main() {
    Users *users = users_get();
    if (users) {
        write_users_to_csv(users, "users.csv");
    } else {
        printf("Erro\n");
    }
    return 0;
}