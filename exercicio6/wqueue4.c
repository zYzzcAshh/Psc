/*
	Programa para simular uma fila de espera

	Objectivo: exemplificar a utilização de uma lista
				não intrusiva duplamente ligada
				e a alocação dinâmica.
*/
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"

/*-----------------------------------------------------------------------------
	Representa um utente na fila de espera
 */
struct user {
	time_t arrival;
	char *name;
};

/*-----------------------------------------------------------------------------
	A fila de espera é representada por estes dois ponteiros
 */
static struct list_node queue = { .next = &queue, .prev = &queue };

/*-----------------------------------------------------------------------------
	Função para insertir um novo utente na fila de espera
 */
static void user_insert(char *name)
{
	struct user *user = malloc(sizeof *user);
	if (NULL == user) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
	user->name = malloc(strlen(name) + 1);
	if (NULL == user->name) {
		free(user);
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
	strcpy(user->name, name);
	time(&user->arrival);

	if (!list_insert_rear(&queue, user)) {
		free(user->name);
		free(user);
	}
}

/*-----------------------------------------------------------------------------
	Função para remover da fila o primeiro utente a ser atendido
 */
static char *user_answer()
{
	if (list_empty(&queue))
		return NULL;

	struct list_node *node = queue.next;
	struct user *user = node->data;
	char *name = user->name;
	free(user);

	list_remove(node);

	return name;
}

/*-----------------------------------------------------------------------------
	Função para remover um utente da fila de espera, por desistencia
 */

bool cmp_name(void *a, void *b) {
	struct user *user = a;
	char *name = b;
	return strcmp(user->name, name) == 0;
}
 			
static void user_remove(char *name)
{
	struct list_node *node = list_search(&queue, cmp_name, name);
	if (node != NULL) {
		struct user *user = list_get_data(node);
		free(user->name);
		free(user);
		list_remove(node);
	}
}

/*-----------------------------------------------------------------------------
	Listar os utentes na fila de espera.
 */
static void print(void *data) {
	struct user *user = data;
	printf("Nome: %s, %ld\n", user->name, time(NULL) - user->arrival);
}

static void user_print()
{
	if (list_empty(&queue)) {
		printf("Fila vazia\n");
		return;
	}
	list_foreach(&queue, print);
/*
	for (struct list_node *node = queue.next; node != &queue; node = node->next) {
		struct user *user = node->data;
		printf("Nome: %s, %ld\n", user->name, time(NULL) - user->arrival);
	}
*/
}

/*-----------------------------------------------------------------------------
	Vazar a lista libertando toda a memória alocada.
 */
static void user_delete_queue()
{
	for (struct list_node *next, *node = queue.next; node != &queue; node = next) {
		next = node->next;
		free(((struct user *)node->data)->name);
		free(node->data);
		free(node);
	}
}

static void help()
{
	printf("Comandos:\n"
		"\tS\t\t- Sair\n"
		"\tN <name> \t- Chegada de novo utente\n"
		"\tD <name>\t- Desistencia de utente\n"
		"\tL\t\t- Listar fila de espera\n"
		"\tA\t\t- Atender utente\n");
}

int main()
{
	char line[100];
	queue.next = queue.prev = &queue;
	while (1) {
		if (fgets(line, sizeof(line), stdin) == NULL)
			return EXIT_FAILURE;
		char *command = strtok(line, " \n");
		if (command == NULL)
			continue;
		char *name = strtok(NULL, " \n");
		switch (command[0]) {
			case 's':
			case 'S':
				user_delete_queue();
				exit(0);
			case 'h':
			case 'H':
				help();
				break;
			case 'n':
			case 'N':
				user_insert(name);
				break;
			case 'd':
			case 'D':
				user_remove(name);
				break;
			case 'l':
			case 'L':
				user_print();
				break;
			case 'a':
			case 'A':
				name = user_answer();
				if (NULL == name)
					printf("Fila vazia\n");
				else {
					printf("Atender %s\n", name);
					free(name);
				}
				break;
		}
	}
}
