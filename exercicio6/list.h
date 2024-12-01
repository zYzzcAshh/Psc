#ifndef LIST_H
#define LIST_H

#include <stdbool.h> // For the bool type

struct list_node {
    struct list_node *next;
    struct list_node *prev;
    void *data;
};

// Function prototypes
bool list_insert_front(struct list_node *node, void *data);
bool list_insert_rear(struct list_node *node, void *data);
void list_remove(struct list_node *node);
bool list_empty(struct list_node *list);
void list_foreach(struct list_node *list, void (*do_it)(void *));
struct list_node *list_search(struct list_node *list, bool (*predicate)(void *, void *), void *context);
void *list_get_data(struct list_node *node);

#endif // LIST_H
