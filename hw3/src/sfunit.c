#include <criterion/criterion.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sfmm.h"

/**
 *  HERE ARE OUR TEST CASES NOT ALL SHOULD BE GIVEN STUDENTS
 *  REMINDER MAX ALLOCATIONS MAY NOT EXCEED 4 * 4096 or 16384 or 128KB
 */

void checkHeaderFooter(void *x, bool isAlloc, int expectedBlockSizeInBytes, int expectedPaddingSize);
void checkFreeHeaderFooter(void *x, int expectedBlockSizeInBytes, int expectedPaddingSize);

void checkHeaderFooter(void *x, bool isAlloc, int expectedBlockSizeInBytes, int expectedPaddingSize){ // if allocated block set isAlloc to 1, else 0
	sf_header *headofx = (sf_header*) (x-8);
	sf_footer *footofx = (sf_footer*) (x - 8 + (headofx->block_size << 4) - 8);
	cr_assert((headofx->alloc) == isAlloc);
    cr_assert(headofx->padding_size == expectedPaddingSize);
    cr_assert(headofx->block_size << 4 == expectedBlockSizeInBytes, 
        "Header block size differs.. Real: %d vs Expected: %d\n", headofx->block_size << 4, expectedBlockSizeInBytes);

	cr_assert((footofx->alloc) == isAlloc);
    cr_assert(footofx->block_size << 4 == expectedBlockSizeInBytes, 
        "Footer block size differs.. Real: %d vs Expectdd: %d\n", headofx->block_size << 4, expectedBlockSizeInBytes);
}

void checkFreeHeaderFooter(void *x, int expectedBlockSizeInBytes, int expectedPaddingSize){
    checkHeaderFooter(x, 0, expectedBlockSizeInBytes, expectedPaddingSize);
}

Test(sf_memsuite, Malloc_an_Integer, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(sizeof(int));
    *x = 4;
    cr_assert(*x == 4, "Failed to properly sf_malloc space for an integer!");
}

Test(sf_memsuite, Free_block_check_header_footer_values, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *pointer = sf_malloc(sizeof(short));
    sf_free(pointer);
    pointer = pointer - 8;
    sf_header *sfHeader = (sf_header *) pointer;
    cr_assert(sfHeader->alloc == 0, "Alloc bit in header is not 0!\n");
    sf_footer *sfFooter = (sf_footer *) (pointer - 8 + (sfHeader->block_size << 4));
    cr_assert(sfFooter->alloc == 0, "Alloc bit in the footer is not 0!\n");
}

Test(sf_memsuite, PaddingSize_Check_char, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *pointer = sf_malloc(sizeof(char));
    pointer = pointer - 8;
    sf_header *sfHeader = (sf_header *) pointer;
    cr_assert(sfHeader->padding_size == 15, "Header padding size is incorrect for malloc of a single char!\n");
}

Test(sf_memsuite, Check_next_prev_pointers_of_free_block_at_head_of_list, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(4);
    memset(x, 0, 4);
    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Coalesce_no_coalescing, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *x = sf_malloc(4);
    void *y = sf_malloc(4);
    memset(y, 0xFF, 4);
    sf_free(x);
    cr_assert(freelist_head == x-8);
    sf_free_header *headofx = (sf_free_header*) (x-8);
    sf_footer *footofx = (sf_footer*) (x - 8 + (headofx->header.block_size << 4)) - 8;

    sf_blockprint((sf_free_header*)((void*)x-8));
    // All of the below should be true if there was no coalescing
    cr_assert(headofx->header.alloc == 0);
    cr_assert(headofx->header.block_size << 4 == 32);
    cr_assert(headofx->header.padding_size == 0);

    cr_assert(footofx->alloc == 0);
    cr_assert(footofx->block_size << 4 == 32);
}

/*
//############################################
// STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
// DO NOT DELETE THESE COMMENTS
//############################################
*/

/* sf_malloc() every path
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Invalid_Size, .init = sf_mem_init, .fini = sf_mem_fini) {
    int askedFor = 0; // Note. when negative value is passed, it is parsed as possible int because of the data type size_t
    void *x = sf_malloc(askedFor);
    cr_assert(x == NULL);
}

Test(sf_memsuite, Malloc_When_Empty_Freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    int askedFor = 80; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);
}

Test(sf_memsuite, Malloc_With_Not_Enough_Freeblock, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(16); checkHeaderFooter(tmp, 1, 32, 0);
    void *tmp2 = sf_malloc(32); checkHeaderFooter(tmp2, 1, 48, 0);
    sf_free(tmp); checkHeaderFooter(tmp, 0, 32, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0); // + 8 because it expects addr of payload

    // Now ask for large space
    int askedFor = 80; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);
}

Test(sf_memsuite, Malloc_With_Enough_Freeblock, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(96); checkHeaderFooter(tmp, 1, 112, 0);

    void *tmp2 = sf_malloc(16);
    sf_blockprint((sf_free_header*)((void*)tmp2-8));
    checkHeaderFooter(tmp2, 1, 32, 0);

    sf_free(tmp); checkHeaderFooter(tmp, 0, 112, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 112, 0); // + 8 because it expects addr of payload

    // Now ask for large space
    int askedFor = 48; // now total of 80 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);
}

Test(sf_memsuite, Malloc_With_Not_Enough_First_Freeblock, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(16); checkHeaderFooter(tmp, 1, 32, 0);
    void *tmp2 = sf_malloc(32); checkHeaderFooter(tmp2, 1, 48, 0);
    sf_free(tmp); checkHeaderFooter(tmp, 0, 32, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0); // + 8 because it expects addr of payload

    // Now ask for large space
    int askedFor = 80; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);
}


/* sf_sbrk() * 4 related tests
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Ask_Max_Bytes_Success, .init = sf_mem_init, .fini = sf_mem_fini) {
	int askedFor = 16368;
	void *x = sf_malloc(askedFor);
	cr_assert(x != NULL);
	
	// sf_blockprint((sf_free_header*)((void*)x-8));

	// All of the below should be true if there was no coalescing
	checkHeaderFooter(x, 1, 16384, 0);
}


Test(sf_memsuite, Ask_Over_Max_Bytes_Fail, .init = sf_mem_init, .fini = sf_mem_fini) {
    int MAX_BYTES = 16369;
    for(int i = 0; i < 1; i++){
        void *x = sf_malloc(MAX_BYTES + i);
        cr_assert(x == NULL);
        if(x != NULL){
            sf_free(x);
        }
    }
}

#include <time.h>
Test(sf_memsuite, Ask_Over_Max_After_Mallocs, .init = sf_mem_init, .fini = sf_mem_fini) {
	int MAX_BYTES = 16368;
	srand(time(0));
	int init_bytes = rand() % 4096;

	// malloc random number of bytes
	void *x = sf_malloc(init_bytes);
	cr_assert(x != NULL);
	sf_header *headofx = (sf_header*) (x-8);
	
	// malloc y, expected to fill up the remaining 4 pages.
	void *y = sf_malloc(MAX_BYTES - (headofx->block_size << 4) - SF_FOOTER_SIZE - SF_HEADER_SIZE);
	cr_assert(y != NULL);

	// free y
	sf_free(y);

	// malloc y
	y = sf_malloc(MAX_BYTES - (headofx->block_size << 4) - SF_FOOTER_SIZE - SF_HEADER_SIZE + 17);
	cr_assert(y == NULL);
}

/* Check Splinter
///////////////////////////////////////////////////////////////////////////////
*/

// Test(sf_memsuite, Check_If_Splinter_Is_Created, .init = sf_mem_init, .fini = sf_mem_fini) {
//     void *x = sf_malloc(24);
//     cr_assert(x != NULL);
//     checkHeaderFooter(x, 1, 24);

//     void *y = sf_malloc(16);
//     cr_assert(y != NULL);
//     checkHeaderFooter(y, 1, 16);

//     sf_free(x);
//     checkHeaderFooter(x, 0, 24);

//     x = sf_malloc(8);
//     cr_assert(x != NULL);
//     checkHeaderFooter(x, 1, 8);

//     sf_snapshot(true);
// }