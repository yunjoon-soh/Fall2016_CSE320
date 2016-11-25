// tree_func.c
#include "tree_func.h"

void add(struct tree_root *r, int key, int value){
	struct node *match = (struct node*) find(r, key);
	if(match->key != key){
		// initialize new entry
		struct node* newNode = (struct node*) malloc(sizeof(struct node));
		r->N++;

		newNode->left = NULL;
		newNode->right = NULL;
		newNode->parent = NULL;
		newNode->key = key;
		newNode->value = value;

		// add depending on the key comparison
		if(match->key > key){
			// assert left is null
			if(match->left != NULL) error("Overriding left child!\n");
			match->left = newNode;
		} else{
			// assert right is null
			if(match->right != NULL) error("Overriding right child!\n");
			match->right = newNode;
		}
		newNode->parent = match;
	}
	else { // match found
		match->value += value;
	}
}

// returns last comparison if key is not found
struct node*find(struct tree_root *r, int key){
	return recurse(key, r->root);
}

struct node*recurse(int key, struct node *now){
	if(now->key == key || now == NULL)
		return now;
	else if(now->key > key)
		return (now->left != NULL)?recurse(key, now->left):now;	
	else
		return (now->right != NULL)?recurse(key, now->right):now;
}

void freeAll(struct tree_root *r){
	struct node* Q[r->N * 2];
	int qs = 0; // q start
	int qe = 0; // q end

	Q[qs++] = r->root;
	while(qs < qe){
		struct node* now = Q[qe++];

		// add child to queue
		if(now->left != NULL)
			Q[qs++] = now->left;
		if(now->right != NULL)
			Q[qs++] = now->right;

		// free the malloced memory
		free(now);
	}
}