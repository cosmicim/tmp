#include <stdlib.h>

struct node {
	int cfd;
	struct node *prev, *next;
};

struct client {
	int cfd;		// socket fd
	char username[20];	// username
	int unique;		// unique ID
	struct client *prev, *next;
};

struct list {
	//struct node *head, *tail;
	struct client *head, *tail;
};

/*static void __list_add(struct node *new,
		struct node *prev,
		struct node *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}*/

struct client *list_add(struct list *l, int cfd)
{
	//struct node *n = malloc(sizeof(struct node));	
	struct client *c = malloc(sizeof(struct client));	

	c->cfd = cfd;
	c->next = c->prev = NULL;

	if (l->tail == NULL) {
		l->head = l->tail = c;
	} else {
		l->tail->next = c;
		c->prev = l->tail;
		l->tail = c;
	}
	return c;
}

void list_add_client(struct list *l, struct client *c)
{
	if (l->tail == NULL) {
		l->head = l->tail = c;
	} else {
		l->tail->next = c;
		c->prev = l->tail;
		l->tail = c;
	}
}

/*
void list_add_first(struct list *l, int cfd)
{
	struct node *n = malloc(sizeof(struct node));
	
	n->cfd = cfd;
	n->next = n->prev = NULL;

	__list_add(n, l->head, l->head->next);
}*/

void list_del(struct list *l)
{
	if (l->head == l->tail) {
		free(l->head);
		l->head = l->tail = NULL;
	} else {
		struct client *c;	
		c = l->head;
		l->head = c->next;
		free(c);
	}
}

void list_del_node(struct list *l, struct client *n)
{
	if (l->head == n && l->tail == n) {
		l->head = l->tail = NULL;
	} else if (n->prev == NULL && n->next != NULL) { // head of the list
		n->next->prev = NULL;
		l->head = n->next;
	} else if (n->next == NULL && n->prev != NULL) { // tail of the list
		n->prev->next = NULL;
		l->tail = n->prev;
	} else {
		n->next->prev = n->prev;
		n->prev->next = n->next;
	}
	free(n);
}

#define list_for_each(pos, head) \
	for (pos = head; pos != NULL; pos = pos->next)
