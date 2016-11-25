// list.c
#include "list.h"

void add(struct list **head, int key, int value){
	if(*head == NULL){
		debug("Head is null, create new one!\n");
		*head = createNode(key, value);
		return;
	}

	struct list *match = find(*head, key);
	if(match == NULL){
		debug("No match found, create new one!\n");
		struct list *now = createNode(key, value);
		now->next = *head;
		*head = now;
	} else {
		debug("Found, updated from %d to ", match->value);
		match->value += value;
		debug("%d!\n", match->value);
	}
}

struct list* createNode(int key, int value){
	debug("Creating new node!\n");
	struct list *new=(struct list*) malloc(sizeof(struct list));
	new->next = NULL;
	new->key = key;
	new->value = value;

	return new;
}

struct list* find(struct list *head, int key){
	if(head != NULL){ // if head is not NULL
		struct list *now = head;
		do{ // while there is sth next
			if(now->key == key) // check if key found
				return now;
		}while( (now = now->next) );
	}

	return NULL; // if no match return NULL
}

void freeAll(struct list **head){
	if(head == NULL)
		return;

	struct list *now = *head;
	struct list *next;
	do{
		next = now->next; // save next head before free
		free(now); // free head
		now = next; // new head is next
	}while(now != NULL); // if now is NULL, done

	*head = NULL;
}
