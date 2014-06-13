#include <stdio.h>
#include "list.h"

int main(int argc, char *argv[])
{
	struct list list;
	list.head = list.tail = NULL;
	list_add(&list, 1337);
	list_add(&list, 666);
	list_add(&list, 69);

	list_del(&list);
	for (struct node *n = list.head; n != NULL; n = n->next) {
		printf("%d\n", n->cfd);
		if (n->cfd == 69)
			list_del_node(&list, n);
	}

	for (struct node *n = list.head; n != NULL; n = n->next) {
		printf("%d\n", n->cfd);
		list_del_node(&list, n);
	}

	for (struct node *n = list.head; n != NULL; n = n->next)
		printf("%d\n", n->cfd);
	
	list_add(&list, 1);
	list_add(&list, 2);
	list_add(&list, 3);
	list_add(&list, 4);
	list_add(&list, 5);

	for (struct node *n = list.head; n != NULL; n = n->next)
		printf("%d\n", n->cfd);

	return 0;
}
