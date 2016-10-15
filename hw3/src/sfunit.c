#include <criterion/criterion.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sfmm.h"

#include <time.h>
#include <errno.h>

/**
 *  HERE ARE OUR TEST CASES NOT ALL SHOULD BE GIVEN STUDENTS
 *  REMINDER MAX ALLOCATIONS MAY NOT EXCEED 4 * 4096 or 16384 or 128KB
 */
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
void checkFreelist();
void checkHeaderFooter(void *x, bool isAlloc, int expectedBlockSizeInBytes, int expectedPaddingSize);
void checkInfo(info* i_f, size_t internal, size_t external, size_t allocations, size_t frees, size_t coalesce);

void checkFreelist(){
    sf_free_header *now = freelist_head;
    cr_assert(now->prev == NULL);

    while(now != NULL){
        cr_assert(now->header.alloc == 0);
        cr_assert(( (sf_footer *)( (char*)now + ( (now->header.block_size << 4) - 8) ) )->alloc == 0);
        now = now->next;
    }
}

void checkHeaderFooter(void *x, bool isAlloc, int expectedBlockSizeInBytes, int expectedPaddingSize){ // if allocated block set isAlloc to 1, else 0
    cr_assert(x != NULL);
    sf_header *headofx = (sf_header*) (x-8);
    sf_footer *footofx = (sf_footer*) (x - 8 + (headofx->block_size << 4) - 8);

    cr_assert((headofx->alloc) == isAlloc,
        "Header alloc differs.. Read:%d vs Expected:%d (Note. expectedBlockSizeInBytes:%d)\n", (headofx->alloc), isAlloc, expectedBlockSizeInBytes);
    cr_assert(headofx->padding_size == expectedPaddingSize,
        "Header padd size differs.. Real: %d vs Expected: %d\n", headofx->padding_size, expectedPaddingSize);
    cr_assert(headofx->block_size << 4 == expectedBlockSizeInBytes, 
        "Header block size differs.. Real: %d vs Expected: %d\n", headofx->block_size << 4, expectedBlockSizeInBytes);

    cr_assert((footofx->alloc) == isAlloc, 
        "Footer alloc differs.. Read:%d vs Expected:%d (Note. expectedBlockSizeInBytes:%d)\n", (footofx->alloc), isAlloc, expectedBlockSizeInBytes);
    cr_assert(footofx->block_size << 4 == expectedBlockSizeInBytes, 
        "Footer block size differs.. Real: %d vs Expected: %d\n", footofx->block_size << 4, expectedBlockSizeInBytes);
}

void checkInfo(info* i_f, size_t internal, size_t external, size_t allocations, size_t frees, size_t coalesce){
    cr_assert(i_f->internal == internal, "Internal: Real: %lu vs Expected: %lu\n", i_f->internal, internal);
    cr_assert(i_f->external == external, "external: Real: %lu vs Expected: %lu\n", i_f->external, external);
    cr_assert(i_f->allocations == allocations, "allocations: Real: %lu vs Expected: %lu\n", i_f->allocations, allocations);
    cr_assert(i_f->frees == frees, "frees: Real: %lu vs Expected: %lu\n", i_f->frees, frees);
    cr_assert(i_f->coalesce == coalesce, "coalesce: Real: %lu vs Expected: %lu\n", i_f->coalesce, coalesce);
}

/* sf_malloc() every path
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Invalid_Size, .init = sf_mem_init, .fini = sf_mem_fini) {
    int askedFor = 0; // Note. when negative value is passed, it is parsed as possible int because of the data type size_t
    void *x = sf_malloc(askedFor);
    cr_assert(x == NULL);
    cr_assert(errno == EINVAL);
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

Test(sf_memsuite, Malloc_With_Not_Enough_First_Page, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(4048); checkHeaderFooter(tmp, 1, 4064, 0); // 4096 - 32 - 16
    void *tmp2 = sf_malloc(15); checkHeaderFooter(tmp2, 1, 32, 1);
    sf_free(tmp2); checkHeaderFooter(tmp2, 0, 32, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0); // + 8 because it expects addr of payload

    // Now ask for large space
    int askedFor = 4096 + 16; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);
}

Test(sf_memsuite, Malloc_With_Not_Enough_First_Page_AvoidSplinter, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(4048); checkHeaderFooter(tmp, 1, 4064, 0); // 4096 - 32 - 16
    void *tmp2 = sf_malloc(15); checkHeaderFooter(tmp2, 1, 32, 1);
    sf_free(tmp2); checkHeaderFooter(tmp2, 0, 32, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0); // + 8 because it expects addr of payload

    // Now ask for large space
    int askedFor = 4096; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes + 16, expectedPaddingSize); // another 16 was added to avoid splinter
}

Test(sf_memsuite, Malloc_With_Not_Enough_Second_Page, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(4048 + 4096); checkHeaderFooter(tmp, 1, 4064 + 4096, 0); // 4096 - 32 - 16
    void *tmp2 = sf_malloc(15); checkHeaderFooter(tmp2, 1, 32, 1);
    sf_free(tmp2); checkHeaderFooter(tmp2, 0, 32, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0); // + 8 because it expects addr of payload
    
    // Now ask for large space
    int askedFor = 4096 + 16; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);
}

Test(sf_memsuite, Malloc_With_Not_Enough_Second_Page_AvoidSplinter, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(4048 + 4096); checkHeaderFooter(tmp, 1, 4064 + 4096, 0); // 4096 - 32 - 16
    void *tmp2 = sf_malloc(15); checkHeaderFooter(tmp2, 1, 32, 1);
    sf_free(tmp2); checkHeaderFooter(tmp2, 0, 32, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0); // + 8 because it expects addr of payload

    // Now ask for large space
    int askedFor = 4096; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes + 16, expectedPaddingSize); // another 16 was added to avoid splinter
}

Test(sf_memsuite, Malloc_With_Not_Enough_Third_Page, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(4048 + 4096 * 2); checkHeaderFooter(tmp, 1, 4064 + 4096 * 2, 0); // 4096 - 32 - 16
    void *tmp2 = sf_malloc(15); checkHeaderFooter(tmp2, 1, 32, 1);
    sf_free(tmp2); checkHeaderFooter(tmp2, 0, 32, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0); // + 8 because it expects addr of payload
    
    // Now ask for large space
    int askedFor = 4096 + 16; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);
}

Test(sf_memsuite, Malloc_With_Not_Enough_Third_Page_AvoidSplinter, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *tmp = sf_malloc(4048 + 4096 * 2); checkHeaderFooter(tmp, 1, 4064 + 4096 * 2, 0); // 4096 - 32 - 16
    void *tmp2 = sf_malloc(15); checkHeaderFooter(tmp2, 1, 32, 1);
    sf_free(tmp2); checkHeaderFooter(tmp2, 0, 32, 0);

    // check freelist
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0); // + 8 because it expects addr of payload

    // Now ask for large space
    int askedFor = 4096; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes + 16, expectedPaddingSize); // another 16 was added to avoid splinter
}

/* Splinter related tests
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Check_If_Splinter_Is_Created, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *x = sf_malloc(24); checkHeaderFooter(x, 1, 24 + 16 + 8, 8);
    void *y = sf_malloc(16); checkHeaderFooter(y, 1, 16 + 16 + 0, 0);
    sf_free(x); checkHeaderFooter(x, 0, 24 + 16 + 8, 0);

    x = sf_malloc(8); checkHeaderFooter(x, 1, 8 + 16 + 8 + 16, 8);
    // 8 : payload, 16: h/f, 8: pad, 16: avoid Splinters
}

/* sf_sbrk() * 4 related tests
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Ask_Max_Bytes_Success, .init = sf_mem_init, .fini = sf_mem_fini) {
	int askedFor = 16368;
	void *x = sf_malloc(askedFor);
	cr_assert(x != NULL);

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

/* sf_free() : invalid ptr
///////////////////////////////////////////////////////////////////////////////
*/

Test(sf_memsuite, Check_Invalid_Ptr_SF_realloc_Already_Freed, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    sf_free(x);
    cr_assert(errno == 0);
    sf_realloc(x, 10); // pass invalid pointer
    cr_assert(errno == EINVAL);
}

Test(sf_memsuite, Check_Invalid_Ptr_SF_Free_Not_Mult_Of_16, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    sf_free((char*) x + 3); // pass invalid pointer

    // same test as before sf_free(), i.e., n/a change
    checkHeaderFooter(x, 1, 1 + 16 + 15, 15);
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);
}

Test(sf_memsuite, Check_Invalid_Ptr_SF_Free_Before_Heap, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    sf_free((char*) x - 16); // pass invalid pointer

    // same test as before sf_free(), i.e., n/a change
    checkHeaderFooter(x, 1, 1 + 16 + 15, 15);
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);
}

Test(sf_memsuite, Check_Invalid_Ptr_SF_Free_After_Heap, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    sf_free((char*) x + 4096); // pass invalid pointer

    // same test as before sf_free(), i.e., n/a change
    checkHeaderFooter(x, 1, 1 + 16 + 15, 15);
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);
}

/* sf_free() : freelist
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Check_If_FreeList_Created_From_First_Malloc, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size
    cr_assert(freelist_head != NULL);

    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);
}

Test(sf_memsuite, Check_If_FreeList_Created_From_Two_Malloc_And_First_One_Freed, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0); // current head

    void *y = sf_malloc(2); checkHeaderFooter(y, 1, 2 + 16 + 14, 14); // payload, h/f, pad

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0); // current head remains the same

    sf_free(x); checkHeaderFooter(x, 0, 1 + 16 + 15, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 1 + 16 + 15, 0); // current head updated
}

/* sf_free() : Different types of coalescing
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Coalesce_alloc_alloc, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(4); checkHeaderFooter(y, 1, 4 + 16 + 12, 12); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    void *z = sf_malloc(12); checkHeaderFooter(z, 1, 12 + 16 + 4, 4);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32 - 32, 0);

    sf_free(y);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -32 -32, 0);    
}

Test(sf_memsuite, Coalesce_alloc_free, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(4); checkHeaderFooter(y, 1, 4 + 16 + 12, 12); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    sf_free(y); // x(alloc) -> y(free) ->freelist_head(free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)y - 8), "freelist_head %p vs y %p\n", freelist_head, (char*)y - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);
}

Test(sf_memsuite, Coalesce_free_alloc, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(4); checkHeaderFooter(y, 1, 4 + 16 + 12, 12); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    void *z = sf_malloc(33); checkHeaderFooter(z, 1, 33 + 16 + 15, 15);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 -32 -32 -64, 0);

    sf_free(x); // x(free) -> y(alloc) -> z(alloc) ->freelist_head(free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -32 -64, 0);

    sf_free(y); // x(free) -> y(free) -> z(alloc) ->freelist_head(free)
    // Coalesc => x, y (free) -> z(alloc) -> freelist_head(free);

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 64, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -32 -64, 0);
}

Test(sf_memsuite, Coalesce_free_free, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(4); checkHeaderFooter(y, 1, 4 + 16 + 12, 12); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    sf_free(x); // x(free) -> y(alloc) -> freelist_head(free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -32, 0);

    sf_free(y); // x(free) -> y(free) ->freelist_head(free)
    // Coalesc => x, y, freelist_head (free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096, 0);
    cr_assert( freelist_head->next == NULL );
    cr_assert( freelist_head->prev == NULL );
}

/* sf_realloc() : Edge cases
///////////////////////////////////////////////////////////////////////////////
*/

Test(sf_memsuite, Realloc_NULL, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    int askedFor = 80; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_realloc(NULL, askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);
}

Test(sf_memsuite, Realloc_NULL_Size_0, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    int askedFor = 0; // now total of 96 bytes in block
    void *x = sf_realloc(NULL, askedFor);
    cr_assert(x == NULL);
}

Test(sf_memsuite, Realloc_Invalid_Address, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    int askedFor = 80; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);

    void *y = sf_realloc((char*)x + 3, askedFor);
    cr_assert(y == NULL);
}

// /* sf_realloc() : Shrinking
// ///////////////////////////////////////////////////////////////////////////////
// */
Test(sf_memsuite, Realloc_alloc_alloc_shrink, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(84); checkHeaderFooter(y, 1, 84 + 16 + 12, 12); // 112: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112, 0);

    void *z = sf_malloc(64); checkHeaderFooter(z, 1, 64 + 16 + 0, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112 - 80, 0);

    void *y2 = sf_realloc(y, 16); // 64 -> 16

    // z will have padding 15, block_size 32
    // free block will be added, size of 48

    cr_assert(y == y2);
    checkHeaderFooter(y2, 16, 32, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 112 - 32, 0);
    cr_assert(freelist_head->next != NULL);
    cr_assert(freelist_head->prev == NULL);

    checkHeaderFooter((char*)freelist_head->next + 8, 0, 4096 - 32 -112 - 80, 0);
}

Test(sf_memsuite, Realloc_alloc_free_shrink, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(4); checkHeaderFooter(y, 1, 4 + 16 + 12, 12); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    void *z = sf_malloc(64); checkHeaderFooter(z, 1, 64 + 16 + 0, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32 - 80, 0);

    void *z2 = sf_realloc(z, 1); // 64 -> 1

    // z will have padding 15, block_size 32
    // free block will be added, size of 48

    cr_assert(z == z2);
    checkHeaderFooter(z2, 1, 32, 15);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 -32 - 32, 0);
    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Realloc_free_alloc_shrink, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(80); checkHeaderFooter(y, 1, 80 + 16 + 0, 0); // 96: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 96 - 32, 0);

    void *z = sf_malloc(33); checkHeaderFooter(z, 1, 33 + 16 + 15, 15);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 -32 -96 -64, 0);

    sf_free(x); // x(free) -> y(alloc) -> z(alloc) ->freelist_head(free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -96 -64, 0);

    void *y2 = sf_realloc(y, 1); // x(free) -> y(free) -> z(alloc) ->freelist_head(free)
    // Coalesc => x, y (free) -> z(alloc) -> freelist_head(free);

    // 32, 96, 64, FREE -> 32, 32, (96-32), 64, FREE

    cr_assert(y == y2);
    checkHeaderFooter(y2, 1, 32, 15);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 96 - 32, 0);
    
    cr_assert(freelist_head->next == (sf_free_header *)((char*)x - 8) );
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 32, 0); // x
    checkHeaderFooter((char*)(freelist_head->next->next) + 8, 0, 4096 -32 -96 -64, 0);
}

Test(sf_memsuite, Realloc_free_free_shrink, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(77); checkHeaderFooter(y, 1, 77 + 16 + 3, 3); // 96: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 96, 0);

    sf_free(x); // x(free) -> y(alloc) -> freelist_head(free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -96, 0);

    void *y2 = sf_realloc(y, 1); // x(free) -> y(realloc) ->freelist_head(free)
    // Coalesc => x, y2, freelist_head(larger)(free)

    cr_assert(y == y2);

    checkHeaderFooter(y2, 1, 32, 15);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    cr_assert(freelist_head->next == (sf_free_header *)((char*)x - 8) );
    cr_assert(freelist_head->prev == NULL );
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 32, 0); // x

    cr_assert(freelist_head->next->next == NULL);
}

/* sf_realloc() : Expanding
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Realloc_alloc_alloc_expand, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(84); checkHeaderFooter(y, 1, 84 + 16 + 12, 12); // 112: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112, 0);

    void *z = sf_malloc(64); checkHeaderFooter(z, 1, 64 + 16 + 0, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112 - 80, 0);

    void *originalFH = freelist_head;
    void *y2 = sf_realloc(y, 100); // 64 -> 100 (32 * 4 + 4) -> padding 12

    // z will have padding 15, block_size 32
    // free block will be added, size of 48

    cr_assert(y != y2);    
    cr_assert(y2 == ((char*)originalFH + 8), "y2=%p vs originalFH=%p\n", y2, (char*)originalFH + 8);
    checkHeaderFooter(y2, 16, 100 + 16 + 12, 12);

    cr_assert(freelist_head != NULL);
    cr_assert(freelist_head == (sf_free_header*)((char*)y - 8));
    checkHeaderFooter((char*)freelist_head + 8, 0, 112, 0); // original y

    cr_assert(freelist_head->next != NULL);
    cr_assert(freelist_head->prev == NULL);
    checkHeaderFooter((char*)freelist_head->next + 8, 0, 4096 - 32 -112 - 80 - 128, 0);

    cr_assert(freelist_head->next->next == NULL);
    cr_assert(freelist_head->next->prev == freelist_head);
}

Test(sf_memsuite, Realloc_alloc_free_expand, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(4); checkHeaderFooter(y, 1, 4 + 16 + 12, 12); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    void *z = sf_malloc(64); checkHeaderFooter(z, 1, 64 + 16 + 0, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32 - 80, 0);

    sf_free(z);

    cr_assert(freelist_head == (sf_free_header *)((char*)z - 8));
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);
    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);

    /*alloc_alloc_free*/
    void *y2 = sf_realloc(y, 65); // 4 -> 65
    // x(alloc) -> y(alloc) -> freelist_head

    // y2 will have padding 15, block_size 96
    // free block will be added, size of 4096 - 32 - 32 - 80
    cr_assert(y == y2);
    checkHeaderFooter(y2, 1, 96, 15);

    cr_assert(freelist_head != NULL); // validate freelist_head
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 96, 0);

    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Realloc_free_alloc_expand, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(80); checkHeaderFooter(y, 1, 80 + 16 + 0, 0); // 96: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 96 - 32, 0);

    void *z = sf_malloc(33); checkHeaderFooter(z, 1, 33 + 16 + 15, 15);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 -32 -96 -64, 0);

    sf_free(x); // x(free) -> y(alloc) -> z(alloc) ->freelist_head(free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -96 -64, 0);

    void *oldFreeList_head_next = freelist_head->next;
    void *y2 = sf_realloc(y, 142); 
    // x(free) -> y(free) -> z(alloc) -> y2(realloc) -> freelist_head(free)
    // x, y(free) -> z(alloc) -> y2(realloc) -> freelist_head(free)
    // 32, 96, 64, FREE -> 128, 64, 160, REST

    cr_assert(y != y2);
    cr_assert(((char*)y2 - 8) == oldFreeList_head_next);
    checkHeaderFooter(y2, 1, 160, 2);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 128, 0);

    cr_assert(freelist_head->next == (sf_free_header*)( (char*)y2 + 160 - 8));
    cr_assert(freelist_head->prev == NULL);
    
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096-128-64-160, 0); // REST
    cr_assert(freelist_head->next->next == NULL);
    cr_assert(freelist_head->next->prev == (sf_free_header*)((char*) x - 8));
}

Test(sf_memsuite, Realloc_free_free_expand, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(4); checkHeaderFooter(y, 1, 4 + 16 + 12, 12); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    void *z = sf_malloc(64); checkHeaderFooter(z, 1, 64 + 16 + 0, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32 - 80, 0);

    sf_free(x); // x(free) -> y(alloc) -> z(alloc) -> freelist_head(free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -32 - 80, 0);

    void *oldFreeList_head_next = freelist_head->next;
    sf_free(y); // x(free) -> y(free) -> z(alloc) -> freelist_head(free)
    // Coalesc => x, y -> z(alloc) -> freelist_head (free)

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32 + 32, 0);
    cr_assert( freelist_head->next == oldFreeList_head_next);
    cr_assert( freelist_head->prev == NULL );

    /*Realloc begin*/
    void *z2 = sf_realloc(z, 100); // 100 + 16 + 12
    
    cr_assert(z == z2);
    checkHeaderFooter(z2, 1, 128, 12);

    cr_assert(freelist_head != NULL);
    cr_assert(freelist_head == (void*)((char*)z2 - 8 + 128));
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32 - 128, 0);
    cr_assert(freelist_head->next == (sf_free_header*)((char*)x - 8) );
    cr_assert(freelist_head->prev == NULL);

    checkHeaderFooter((char*)freelist_head->next + 8, 0, 32 + 32, 0);
    cr_assert(freelist_head->next->next == NULL);
    cr_assert(freelist_head->next->prev == freelist_head);
}

/* sf_realloc() : Shrinking try avoiding splinter
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Realloc_alloc_alloc_shrink_avoid_splinter, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(96); checkHeaderFooter(y, 1, 96 + 16 + 0, 0); // 112: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112, 0);

    void *z = sf_malloc(64); checkHeaderFooter(z, 1, 64 + 16 + 0, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112 - 80, 0);

    /*realloc begins*/
    void *y2 = sf_realloc(y, 96 - 16); // 96 -> 80

    // just normal shrinking will cause splinter
    // y2 will have padding 0, block_size 64 + 16 + 0 + 16(avoid splinter)
    // x(alloc) -> y(realloc) -> z(alloc) -> REST

    cr_assert(y == y2);
    checkHeaderFooter(y2, 1, 80 + 16 + 16, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112 - 80, 0);
    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);

    void *oldFreeList_head = freelist_head;
    void *y3 = sf_realloc(y, 96 - 32); // 96 -> 64

    // just normal shrinking will not cause splinter
    // y3 will have padding 0, block_size 64 + 16 + 0

    cr_assert(y2 == y3);
    checkHeaderFooter(y3, 1, 64 + 16 + 0, 0);

    cr_assert(freelist_head == (sf_free_header*)((char*)y3 + 80 - 8) );
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    cr_assert(freelist_head->next == oldFreeList_head);
    cr_assert(freelist_head->prev == NULL);

    cr_assert(freelist_head->next->next == NULL);
    cr_assert(freelist_head->next->prev == freelist_head);
}

/* sf_realloc() : Expanding try avoiding splinter
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Realloc_alloc_alloc_expand_avoid_splinter, .init = sf_mem_init, .fini = sf_mem_fini) {
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(96); checkHeaderFooter(y, 1, 96 + 16 + 0, 0); // 112: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112, 0);

    void *z = sf_malloc(64); checkHeaderFooter(z, 1, 64 + 16 + 0, 0);

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112 - 80, 0);

    sf_free(y); checkHeaderFooter(y, 0, 96 + 16 + 0, 0);

    cr_assert(freelist_head == (sf_free_header*)( (char*) y - 8 ));
    checkHeaderFooter((char*)freelist_head + 8, 0, 112, 0);

    /*realloc begins*/
    void *x2 = sf_realloc(x, 32 + 112 - 16 - 16); // 96 -> 112
    // 112 because 32 + 112 - 16(header) - 16(splinter)

    cr_assert(x == x2);
    checkHeaderFooter(x2, 1, 32 + 112, 0);

    cr_assert(freelist_head == (sf_free_header*)((char*)z + 80 - 8));
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 112 - 80, 0);
    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);
}

/* sf_realloc() : Expanding Edge Case
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Realloc_alloc_free_expand_internal_malloc_call, .init = sf_mem_init, .fini = sf_mem_fini) {
//     cr_assert(freelist_head == NULL);
    void *x = sf_malloc(4096 - 16 - 64); checkHeaderFooter(x, 1, 4096 - 64, 0); // 4032: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 64, 0);

    void *y = sf_malloc(16); checkHeaderFooter(y, 1, 16 + 16 + 0, 0); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 4032 - 32, 0); // 32: FREE b_size

    /*realloc begins*/
    // Before: x(alloc)->y(alloc)->FREE
    void *oldSbrk = sf_sbrk(0);
    void *y2 = sf_realloc(y, 4096 + 32 + 32 - 16); // 4160 - 16: b_size, tight realloc
    // y + FREE + new page
    // After:  x(alloc)->y(realloc)->NO FREE

    cr_assert(y == y2);
    checkHeaderFooter(y2, 1, 4096 + 32 + 32, 0);

    cr_assert(freelist_head == NULL);
    cr_assert(oldSbrk == sf_sbrk(0) - 4096);
}

/* sf_info() : 
///////////////////////////////////////////////////////////////////////////////
*/
Test(sf_memsuite, Info_Malloc_When_Empty_Freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
    
    cr_assert(freelist_head == NULL);
    int askedFor = 80; // now total of 96 bytes in block
    int expectedPaddingSize = 0;
    int expectedBlockSizeInBytes = 16 + expectedPaddingSize + askedFor;
    void *x = sf_malloc(askedFor);
    cr_assert(x != NULL);

    checkHeaderFooter(x, 1, expectedBlockSizeInBytes, expectedPaddingSize);

    info i_f;
    sf_info(&i_f);
    checkInfo(&i_f, 16 + expectedPaddingSize, 4096 - expectedBlockSizeInBytes, 1, 0, 0);
}

Test(sf_memsuite, Info_Check_If_Splinter_Is_Created, .init = sf_mem_init, .fini = sf_mem_fini) {
    info i_f;
    void *x = sf_malloc(24); checkHeaderFooter(x, 1, 24 + 16 + 8, 8);
    void *y = sf_malloc(16); checkHeaderFooter(y, 1, 16 + 16 + 0, 0);

    sf_info(&i_f); checkInfo(&i_f, 16 + 8 + 16 + 0, 4096 - 48 - 32, 2, 0, 0);

    sf_free(x); checkHeaderFooter(x, 0, 24 + 16 + 8, 0);

    sf_info(&i_f); checkInfo(&i_f, 16 + 0, 48 + 4096 - 48 - 32, 2, 1, 0);

    x = sf_malloc(8); checkHeaderFooter(x, 1, 8 + 16 + 8 + 16, 8);
    // 8 : payload, 16: h/f, 8: pad, 16: avoid Splinters
    
    sf_info(&i_f); checkInfo(&i_f, 16 + 8 + 16 + 0, 4096 - 48 - 32, 3, 1, 0);
}

Test(sf_memsuite, Info_Coalesce_free_free, .init = sf_mem_init, .fini = sf_mem_fini) {
    info i_f;
    cr_assert(freelist_head == NULL);
    void *x = sf_malloc(1); checkHeaderFooter(x, 1, 1 + 16 + 15, 15); // 32: b_size

    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32, 0);

    void *y = sf_malloc(4); checkHeaderFooter(y, 1, 4 + 16 + 12, 12); // 32: b_size
    
    cr_assert(freelist_head != NULL);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096 - 32 - 32, 0);

    sf_free(x); // x(free) -> y(alloc) -> freelist_head(free)
    sf_info(&i_f); checkInfo(&i_f, 16 + 12, 4096 - 32 - 32 + 32, 2, 1, 0);

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 32, 0);
    checkHeaderFooter((char*)(freelist_head->next) + 8, 0, 4096 -32 -32, 0);

    sf_free(y); // x(free) -> y(free) ->freelist_head(free)
    // Coalesc => x, y, freelist_head (free)

    sf_info(&i_f); checkInfo(&i_f, 0, 4096, 2, 2, 1);

    cr_assert(freelist_head != NULL);
    cr_assert((void*)freelist_head == (void*)((char*)x - 8), "freelist_head %p vs x %p\n", freelist_head, (char*)x - 8);
    checkHeaderFooter((char*)freelist_head + 8, 0, 4096, 0);
    cr_assert( freelist_head->next == NULL );
    cr_assert( freelist_head->prev == NULL );
}

Test(sf_memsuite, Info_Invalid_Size, .init = sf_mem_init, .fini = sf_mem_fini) {
    info i_f;
    int askedFor = 0; // Note. when negative value is passed, it is parsed as possible int because of the data type size_t
    void *x = sf_malloc(askedFor);
    cr_assert(x == NULL);

    sf_info(&i_f); checkInfo(&i_f, 0, 0, 0, 0, 0);
}