// list.c
#include "list.h"

size_t count_list(struct list *head){
	struct list *now = head;
	size_t cnt = 0;
	while(now != NULL){
		cnt++;
		now = now->next;
	}
	return cnt;
}

void add(struct list **head, int key, int value){
	if(*head == NULL){
		// debug("Head is null, create new one!\n");
		*head = createNode(key, value);
		return;
	}

	struct list *match = find(*head, key);
	if(match == NULL){
		// debug("No match found, create new one!\n");
		struct list *now = createNode(key, value);
		now->next = *head;
		*head = now;
	} else {
		// debug("Found, updated from %d to ", match->value);
		match->value += value;
		// debug("%d!\n", match->value);
	}
}

struct list* createNode(int key, int value){
	// debug("Creating new node!\n");
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
	while(now != NULL){
		next = now->next; // save next head before free
		free(now); // free head
		now = next; // new head is next
	}

	*head = NULL;
}

struct list *find_max(struct list *head){
	struct list *now = head;
	struct list *max = now;

	while(now != NULL){
		if(max->value < now->value)
			max = now;
		else if(max->value == now->value){
			max = (max->key > now->key)?now:max;
		}
		now = now->next;
	}
	return max;
}