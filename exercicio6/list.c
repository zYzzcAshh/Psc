#include <stdlib.h>
#include "list.h"


bool list_insert_front(struct list_node *node, void *data)
{
	return false;
}

bool list_insert_rear(struct list_node *node, void *data)
{
	struct list_node *new = malloc(sizeof *node);
	if (NULL == new) 
		return false;

	new->data = data;

	new->next = node;
	new->prev = node->prev;
	node->prev->next = new;
	node->prev = new;
	return true;
}

void list_remove(struct list_node *node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
	free(node);

}

bool list_empty(struct list_node *list) {
	return list->next == list;
}

void list_foreach(struct list_node *list, void (*do_it)(void *))
{
	for (struct list_node *node = list->next; node != list; node = node->next)
		do_it(node->data);
}

struct list_node *list_search(struct list_node *list,
						bool (*predicate)(void *, void *), void *context)
{
	for (struct list_node *node = list->next; node != list; node = node->next) {
		if (predicate(node->data, context))
			return node;
	}
	return NULL;
}

void *list_get_data(struct list_node *node) {
	return node->data;
}
