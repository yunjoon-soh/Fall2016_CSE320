#include <criterion/criterion.h>

#define COLOR
#include "list.h"

/**
* TEST BEGINS HERE
*/
Test(list_init, add1){
	struct list *root = NULL;

	add(&root, 5, 1);
	
	cr_assert(root->value = 1);
	cr_assert(root->key = 5);
	cr_assert(root->next == NULL);

	freeAll(&root);

	cr_assert(root == NULL);
}

Test(list_init, add2){
	struct list *root = NULL;

	add(&root, 1, 1);
	
	cr_assert(root->value = 1);
	cr_assert(root->key = 1);
	cr_assert(root->next == NULL);

	add(&root, 2, 1);
	
	cr_assert(root->value = 2);
	cr_assert(root->key = 2);
	cr_assert(root->next != NULL);

	cr_assert(root->next->value = 1);
	cr_assert(root->next->key = 1);
	cr_assert(root->next->next == NULL);

	freeAll(&root);

	cr_assert(root == NULL);
}

Test(list_init, add10){
	struct list *root = NULL;

	for(int i = 0; i < 10; i++){
		add(&root, i, i);

		cr_assert(root->value == i);
		cr_assert(root->key == i);
	}

	freeAll(&root);

	cr_assert(root == NULL);
}

Test(list_update, update_root){
	struct list *root = NULL;

	for(int i = 0; i < 10; i++){
		add(&root, i, i);

		cr_assert(root->value == i);
		cr_assert(root->key == i);
	}

	add(&root, 9, 9);

	cr_assert(root->key == 9);
	cr_assert(root->value == 18);

	freeAll(&root);

	cr_assert(root == NULL);
}

Test(list_update, find_test){
	struct list *root = NULL;

	for(int i = 0; i < 10; i++){
		add(&root, i, i);

		cr_assert(root->value == i);
		cr_assert(root->key == i);
	}

	add(&root, 0, 9);

	struct list* match = find(root, 0);

	cr_assert(match->key == 0);
	cr_assert(match->value == 9);
	cr_assert(match->next == NULL);

	freeAll(&root);

	cr_assert(root == NULL);
}