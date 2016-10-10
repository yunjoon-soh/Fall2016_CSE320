#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sfmm.h"

// Colors
#ifdef COLOR
    #define KNRM  "\x1B[0m"
    #define KRED  "\x1B[1;31m"
    #define KGRN  "\x1B[1;32m"
    #define KYEL  "\x1B[1;33m"
    #define KBLU  "\x1B[1;34m"
    #define KMAG  "\x1B[1;35m"
    #define KCYN  "\x1B[1;36m"
    #define KWHT  "\x1B[1;37m"
    #define KBWN  "\x1B[0;33m"
#else
    /* Color was either not defined or Terminal did not support */
    #define KNRM
    #define KRED
    #define KGRN
    #define KYEL
    #define KBLU
    #define KMAG
    #define KCYN
    #define KWHT
    #define KBWN
#endif

#ifdef DEBUG
    #define debug(S, ...)   fprintf(stdout, KMAG "DEBUG: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define error(S, ...)   fprintf(stderr, KRED "ERROR: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL "WARN: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU "INFO: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN "SUCCESS: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
    #define debug(S, ...)
    #define error(S, ...)   fprintf(stderr, KRED "ERROR: " KNRM S, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL "WARN: " KNRM S, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU "INFO: " KNRM S, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN "SUCCESS: " KNRM S, ##__VA_ARGS__)
#endif

/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */

sf_free_header* freelist_head = NULL;

size_t calculatePaddingSize(size_t size);
void *getLastByteAddrOfBlock(sf_free_header* now);

// if no split is needed, NULL returned
// if split is needed, start address of split block is returned
void *needSplit(sf_free_header* now, int numOfNewBytes, int numOfRequiredBytes);


size_t calculatePaddingSize(size_t size){
	if(size % 16 != 0){
		return 16 - (size % 16);
	}

	return 0;
}

void *getLastByteAddrOfBlock(sf_free_header* now){
	return (void*)(((char*) now) + now->header.block_size);
}

// if no split is needed, NULL returned
// if split is needed, start address of split block is returned
void *needSplit(sf_free_header* now, int numOfNewBytes, int numOfRequiredBytes){
	if(numOfNewBytes < numOfRequiredBytes){
		error("needSplit numOfNewBytes(%d) < numOfRequiredBytes(%d)\n", numOfNewBytes, numOfRequiredBytes);
		return NULL;
	}

	if(numOfNewBytes < numOfRequiredBytes + 32){
		return NULL;
	}

	return (void*)(((char*)now) + numOfRequiredBytes);
}


// 1. Parameter check
// 		if the parameter is 0, return NULL pointer

// 2. Check free block
// 2-1. If free block does not exists, get free block
// 2-2.	If free block exists and size is not enough, get some more free block
// 2-3. If free block exists and size is enough, continue

// 3. Allocate to free block
// 3-1. Setup the current free block and remove from the doubly linked list
// 3-2. If allocation divides the block into two, add the second block to freelist
// 3-3. If allocation does not divide the existing block

// 4. Return the correct address

void *sf_malloc(size_t size){
	int numOfNewBytes, numOfRequiredBytes, numOfBlockBytes;
	void *allocAddr, *sbrk_ret, *splitAddr;
	sf_free_header* now;
	size_t paddingSize;
	bool noFit = 0;

	// 1. Parameter check
	debug("1. Parameter check (size = %lu)\n", size);
	if(size <= 0){ // if the parameter is 0, return NULL pointer //TODO whether it is <= or ==
		return NULL;
	}

	paddingSize = calculatePaddingSize(size);
	numOfRequiredBytes = size + SF_HEADER_SIZE + SF_FOOTER_SIZE + paddingSize;

	// 2. Check free block
	debug("2. Check free block\n");
	if(freelist_head == NULL){ 	// if free_header is null, ask for more space
		// new block
		allocAddr = (void *)((char *)sf_sbrk(0)); // location to save the data
		
		// 2-1. If free block does not exists, get free block
		debug("2-1. If free block does not exists, get new page(allocAddr=%p)\n", allocAddr);
		numOfNewBytes = 0;
		while(numOfNewBytes < numOfRequiredBytes){
		// if numOfNewBytes is >=, then we can create a whole new block
			sbrk_ret = sf_sbrk(1); // non-zero value to ask for one page (4096 bytes)
			if(sbrk_ret == (void*)-1){
				return NULL;
			}
			else{
				numOfNewBytes += 4096; // added 4096 more bytes
			}
		}

		((sf_free_header*) allocAddr)->header.alloc = 0;
		((sf_free_header*) allocAddr)->header.block_size = numOfNewBytes >> 4;
		((sf_free_header*) allocAddr)->header.padding_size = 0;
		((sf_free_header*) allocAddr)->next = NULL;
		((sf_free_header*) allocAddr)->prev = NULL;

		sf_footer* footer = (sf_footer*)(((char*)allocAddr) + (((sf_free_header *) allocAddr)->header.block_size << 4)- SF_HEADER_SIZE);
		footer->alloc = 0;
		footer->block_size = numOfNewBytes >> 4;

		freelist_head = ((sf_free_header*) allocAddr);

		now = freelist_head;

		debug("allocAddr=%p\n", allocAddr);
		sf_varprint(allocAddr); // this returns strange value...
		sf_varprint((char*)allocAddr+ 8);
		sf_snapshot(true);
	}
	else { // check the freelist to see if there is a block to allocate size
		now = freelist_head;

		// find the free block
		while(numOfRequiredBytes > (now->header.block_size << 4)){
			if(now->next != NULL) 
				now = now->next;
			else{ // if end of list reached without finding allocatable block
				noFit = 1;
				break;
			}
		}

		if(noFit == 1){ // if end of list reached without finding allocatable block
			debug("2-2.	If free block exists and size is not enough, get some more free block\n");
			if(sf_sbrk(0) == getLastByteAddrOfBlock(now)){ // if last free block is at the end of the boundary, numOfNewBytes is the size of that block
				numOfNewBytes = now->header.block_size;
			}
			else{ // if last free block is not at the end of the heap boundary
				numOfNewBytes = 0;
			}

			while(numOfNewBytes < numOfRequiredBytes){
			// if numOfNewBytes is >=, then we can create a whole new block
				sbrk_ret = sf_sbrk(1); // non-zero value to ask for one page (4096 bytes)
				if(sbrk_ret == (void*)-1){
					return NULL;
				}
				else{
					numOfNewBytes += 4096; // added 4096 more bytes
				}
			}

			now->header.block_size = numOfNewBytes; // create one large chunk

			sf_footer* footer = (sf_footer*)(((char*)now) + (((sf_free_header *) now)->header.block_size << 4)- SF_HEADER_SIZE);
			footer->alloc = 0;
			footer->block_size = numOfNewBytes;

		} else {
			// 2-3. If free block exists and size is enough, continue
			debug("2-3. If free block exists and size is enough, continue\n");
			numOfNewBytes = now->header.block_size << 4;
		}
	}

	// 3. Allocate to free block
	debug("3. Allocate to free block(now=%p, numOfNewBytes=%d)\n", (void*) now, numOfNewBytes);

	// 3-1. Setup the current free block and remove from the doubly linked list
	debug("3-1. Setup the current free block and remove from the doubly linked list\n");

	if(now->prev != NULL && now->next != NULL){
		now->prev->next = now->next;
		now->next->prev = now->prev;
	}
	else if(now->prev == NULL && now->next != NULL){ // head
		freelist_head = now->next;
		now->next->prev = NULL;
	}
	else if(now->prev != NULL && now->next == NULL){ // tail
		now->prev->next = NULL;
		now->next->prev = now->prev;
	}
	else { // only now exists in the link
		freelist_head = NULL;
	}

	// decide whether to pad or split into two blocks
	splitAddr = needSplit(now, numOfNewBytes, numOfRequiredBytes);

	// if splitting, add the free block to start of the list, then return the correct addr.
	if(splitAddr != NULL){ // if split is needed
		// 3-2. If allocation divides the block into two, add the second block to freelist
		debug("3-2. If allocation divides the block into two, add the second block to freelist(splitAddr=%p)\n", splitAddr);

		// set split block
		((sf_free_header *) splitAddr)->header.alloc = 0;
		((sf_free_header *) splitAddr)->header.block_size = (numOfNewBytes - numOfRequiredBytes) >> 4;
		((sf_free_header *) splitAddr)->header.padding_size = 0;
		((sf_free_header *) splitAddr)->prev = NULL;
		((sf_free_header *) splitAddr)->next = freelist_head;

		sf_footer* footer = (sf_footer*)(((char*)splitAddr) + (((sf_free_header *) splitAddr)->header.block_size << 4)- SF_HEADER_SIZE);
		footer->alloc = 0;
		footer->block_size = (numOfNewBytes - numOfRequiredBytes) >> 4;

		sf_varprint((char*)splitAddr + 8);

		// link backwards
		if(freelist_head != NULL)
			freelist_head->prev = (sf_free_header *) splitAddr;

		// set current free lock as the head
		freelist_head = (sf_free_header *) splitAddr;

		numOfBlockBytes = numOfRequiredBytes;
	} else { // if no split is needed
		// 3-3. If allocation does not divide the existing block
		debug("3-3. If allocation does not divide the existing block\n");
		numOfBlockBytes = now->header.block_size;
	}

	// set header
	now->header.alloc = 1;
	now->header.block_size = numOfBlockBytes >> 4;
	now->header.padding_size = paddingSize;

	// set footer
	sf_footer* footer = (sf_footer*)(((char*)now) + numOfBlockBytes - SF_HEADER_SIZE);
	footer->alloc = 1;
	footer->block_size = numOfBlockBytes >> 4;

	// 4. Return the correct address
	debug("4. Return the correct address\n");

	return (void*)(((char*)now) + SF_HEADER_SIZE);
}

void sf_free(void *ptr){
	// check if the ptr is valid pointer
	// if it is invalid, return error??? //TODO check the behaviour of this situation
	// if it is a valid, free the block
	// check for possible coalescing
	// if coalescing, coalesce
	// add to the front of the freelist //TODO check if this is correct
}

void *sf_realloc(void *ptr, size_t size){
	return NULL;
}

int sf_info(info* meminfo){
	return -1;
}
