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
int sf_sbrk_call = 0;

static size_t internal = 0;
static size_t external = 0;
static size_t allocations = 0;
static size_t frees = 0;
static size_t coalesce = 0;

/*********************************************************************************
Prototypes
**********************************************************************************/
size_t calculatePaddingSize(size_t size);
void *getLastByteAddrOfBlock(sf_free_header* now);

// if no split is needed, NULL returned
// if split is needed, start address of split block is returned
void *needSplit(sf_free_header* now, int numOfNewBytes, int numOfRequiredBytes);

bool validatePointer(void *ptr); // return 0 if invalid

/* checkCoalese(..) it also sets before/after free block's header and footer, if applicable
// return 0, if block before and after are alloced
// return 1, if block before is alloced but block after is free
// return 2, if block before is free but block after is alloced
// return 3, if block before and after are free
*/
int checkCoalesce(void *ptr, sf_free_header **bHead, sf_free_header **aHead, sf_footer **bFoot, sf_footer **aFoot);

void fillHeader(void* ptr, int isAlloc, int blockSize, int paddingSize, bool isPayloadAddr);
void *fillFooter(void* headerAddr, int isAlloc, int blockSize);

void addToHead(sf_free_header* newHead);
void removeNodeFromDoublyLinkedList(sf_free_header* target);

void *sf_malloc_internal(size_t size);
void sf_free_internal(void* ptr);

/*********************************************************************************
Helper Functions
**********************************************************************************/
size_t calculatePaddingSize(size_t size){
	if(size % 16 != 0){
		return 16 - (size % 16);
	}

	return 0;
}

void *getLastByteAddrOfBlock(sf_free_header* now){
	return (void*)
		( (char*)now + (now->header.block_size << 4) );
}

/* needSplit(..) check if split is needed
// if split is needed, start address of split block is returned
// if no split is needed, NULL returned
*/
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

void fillHeader(void* ptr, int isAlloc, int blockSize, int paddingSize, bool isPayloadAddr){
	sf_header *header;

	if(isPayloadAddr != 0){ // it is header addr
		header = (sf_header *)( (char*)ptr - SF_HEADER_SIZE );
	}
	else {
		header = (sf_header *)ptr;
	}

	if(isAlloc == 1){
		header->alloc = isAlloc;
		header->block_size = blockSize;
		header->padding_size = paddingSize;
	}	
	else { // (header->alloc == 0){
		((sf_free_header*)header)->header.alloc = isAlloc;
		((sf_free_header*)header)->header.block_size = blockSize;
		((sf_free_header*)header)->header.padding_size = paddingSize;
		((sf_free_header*)header)->next = NULL;
		((sf_free_header*)header)->prev = NULL;
	}
}

void *fillFooter(void* headerAddr, int isAlloc, int blockSize){ 
	sf_footer *footer = (sf_footer *)( (char*)headerAddr + ((blockSize << 4) - SF_FOOTER_SIZE) );
	footer->alloc = isAlloc;
	footer->block_size = blockSize;
	return footer;
}

// check if the ptr is actual pointer that has been assigned
bool validatePointer(void *ptr){  // return 0 if invalid
	if( (unsigned long)ptr % 16 != 0){
		return 0;
	}
	sf_header* header = (sf_header*)((char*)ptr - SF_HEADER_SIZE);
	sf_footer* footer = (sf_footer*)((char*)header + (header->block_size << 4)- SF_FOOTER_SIZE);

	unsigned long superHeapStart = (unsigned long) sf_sbrk(0);
	unsigned long superHeapEnd = superHeapStart - 4096 * sf_sbrk_call;

	if( (unsigned long)header > superHeapStart || (unsigned long)footer > superHeapStart ){
		return 0;
	}

	if( (unsigned long)header < superHeapEnd || (unsigned long)footer < superHeapEnd ){
		return 0;
	}

	debug("validatePointer(%p) Result: header: %p, footer: %p\n", ptr, header, footer);
	debug("block_size: %d vs %d, alloc: %d vs %d\n", header->block_size << 4, footer->block_size << 4, header->alloc, footer->alloc);
	if(header->block_size == footer->block_size && header->alloc==footer->alloc){
		return 1;
	} else {
		return 0;
	}
}

/* checkCoalese(..) it also sets before/after free block's header and footer, if applicable
// return 0, if block before and after are alloced
// return 1, if block before is alloced but block after is free
// return 2, if block before is free but block after is alloced
// return 3, if block before and after are free
*/
int checkCoalesce(void *ptr, sf_free_header **bHead, sf_free_header **aHead, sf_footer **bFoot, sf_footer **aFoot){
	unsigned int beforeAlloc = 0, afterAlloc = 0, coalesceType;
	sf_header * now = (sf_header*)((char*)ptr - SF_HEADER_SIZE);
	*bFoot = (sf_footer*)     ((char*)now - SF_FOOTER_SIZE);
	debug("*bFoot=%p\n", *bFoot);

	*bHead = (sf_free_header*)((char*)(*bFoot) - ((((sf_free_header*)(*bFoot))->header.block_size << 4) - SF_HEADER_SIZE));
	debug("*bHead=%p\n", *bHead);

	*aHead = (sf_free_header*)( (char*)now + ( ((sf_free_header*)now) -> header.block_size << 4) );
	debug("*aHead=%p, now=%p, %d\n", *aHead, now, (((sf_free_header*)now)->header.block_size << 4));

	*aFoot = (sf_footer*)     ((char*)(*aHead) + ((((sf_free_header*)(*aHead))->header.block_size << 4) - SF_FOOTER_SIZE));
	debug("*aFoot=%p\n", *aFoot);

	if((unsigned long)(*bFoot) <= (unsigned long)(sf_sbrk(0) - 4096 * sf_sbrk_call)){
		debug("Before mother god sf_sbrk %p vs %p\n", *bFoot, (char*)(sf_sbrk(0) - 4096 * sf_sbrk_call));
		beforeAlloc = 1; // before mother god alloc, then mark as if previous is already allocated
		bHead=NULL;
		bFoot=NULL;
	}
	else{
		debug("After mother god sf_sbrk %p vs %p\n", *bFoot, (char*)(sf_sbrk(0) - 4096 * sf_sbrk_call));
		beforeAlloc = (*bHead)->header.alloc;
	}

	if((void*)*aHead > sf_sbrk(0)){
		afterAlloc = 1; // treat as after the block is malloced
		aHead=NULL;
		aFoot=NULL;
	} else{
		afterAlloc = (*aHead)->header.alloc;
	}

	if(beforeAlloc == 1){
		if(afterAlloc == 1){
			coalesceType = 0;
		}
		else{
			coalesceType = 1;
		}
	} else{
		if(afterAlloc == 1){
			coalesceType = 2;
		}
		else{
			coalesceType = 3;
		}
	}

	if(coalesceType != 0){
		coalesce += 1;
	}

	return coalesceType;
}

/*freelist_head related*/
void addToHead(sf_free_header* newHead){
	// (*newHead)->next = NULL;
	// (*newHead)->prev = NULL;
	if(freelist_head == NULL){
		freelist_head = newHead;
		return;
	}

	if(freelist_head == newHead){
		return;
	}

	// connect between current head and new head
	freelist_head->prev = newHead;
	newHead->next = freelist_head;	

	// set the new head
	freelist_head = newHead;
}

void removeNodeFromDoublyLinkedList(sf_free_header* now){
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
	}
	else { // only now exists in the link
		if(freelist_head == now)
			freelist_head = NULL;
	}

	now->next = NULL;
	now->prev = NULL;
}

/*********************************************************************************
Main Functions
**********************************************************************************/
/*sf_malloc*/
// 1. Parameter check
// 		if the parameter is 0, return NULL pointer

// 2. Check free block
// 2-1. If free block does not exists, get free block
// 2-2.	If free block exists and size is not enough, get some more free block
// 2-3. If free block exists and size is enough, continue

// 3. Allocate to free block
//    : Setup the current free block and remove from the doubly linked list
// 3-1. If allocation divides the block into two, add the second block to freelist
// 3-2. If allocation does not divide the existing block

// 4. Return the correct address
void *sf_malloc(size_t size){
	void *ret = sf_malloc_internal(size);
	if(ret != NULL){
		allocations += 1;
	}
	return ret;
}

void *sf_malloc_internal(size_t size){
	int numOfNewBytes, numOfRequiredBytes, numOfBlockBytes;
	void *allocAddr, *sbrk_ret, *splitAddr;
	sf_free_header* now;
	size_t paddingSize;
	bool noFit = 0;

	debug("=================================================================================\n");
	debug("1. Parameter check (size = %lu, size in hex: %x)\n", size, (unsigned int)size);
	if(size <= 0){ // if the parameter is 0, return NULL pointer //TODO whether it is <= or ==
		errno = EINVAL;
		return NULL;
	}

	paddingSize = calculatePaddingSize(size);
	numOfRequiredBytes = size + SF_HEADER_SIZE + SF_FOOTER_SIZE + paddingSize;

	debug("2. Check free block numOfRequiredBytes=%d\n", numOfRequiredBytes);
	if(freelist_head == NULL){ 	// if free_header is null, ask for more space

		allocAddr = (void *)((char *)sf_sbrk(0)); // start of new page
		
		debug("2-1. If free block does not exists, get new page(allocAddr=%p)\n", allocAddr);
		numOfNewBytes = 0;
		while(numOfNewBytes < numOfRequiredBytes){
		// if numOfNewBytes is >=, then we can create a whole new block
			sbrk_ret = sf_sbrk(1); // non-zero value to ask for one page (4096 bytes)
			if(sbrk_ret == (void*)-1){
				// error("sf_sbrk return -1\n");
				// errno expected to be set by sf_sbrk correctly
				return NULL;
			}
			else{
				sf_sbrk_call++;
				numOfNewBytes += 4096; // added 4096 more bytes
			}
		}
		
		fillHeader(allocAddr, 0, numOfNewBytes >> 4, 0, 0);
		fillFooter(allocAddr, 0, numOfNewBytes >> 4);
		addToHead(allocAddr);

		now = freelist_head;
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
			void *bigBreak = getLastByteAddrOfBlock(now);
			if(sf_sbrk(0) == bigBreak){ // if last free block is at the end of the boundary, numOfNewBytes is the size of that block
				numOfNewBytes = now->header.block_size << 4;
			}
			else{ // if last free block is not at the end of the heap boundary
				numOfNewBytes = 0;
			}

			while(numOfNewBytes < numOfRequiredBytes){
			// if numOfNewBytes is >=, then we can create a whole new block
				sbrk_ret = sf_sbrk(1); // non-zero value to ask for one page (4096 bytes)
				if(sbrk_ret == (void*)-1){
					// error("sf_sbrk return -1\n");
					// errno expected to be set by sf_sbrk correctly
					return NULL;
				}
				else{
					sf_sbrk_call++;
					numOfNewBytes += 4096; // added 4096 more bytes
				}
			}

			now->header.block_size = numOfNewBytes >> 4; // create one large chunk
			fillFooter(now, 0, numOfNewBytes >> 4);
			//Note. numOfNewBytes already calculated at this line
		} else {
			debug("2-3. If free block exists and size is enough, continue\n");
			numOfNewBytes = now->header.block_size << 4;
		}
	}

	debug("3. Allocate to free block(now=%p, numOfNewBytes=0x%x, %d)\n", (void*) now, numOfNewBytes, numOfNewBytes);

	debug("Setup the current free block and remove from the doubly linked list\n");
	removeNodeFromDoublyLinkedList(now);

	// decide whether to pad or split into two blocks
	splitAddr = needSplit(now, numOfNewBytes, numOfRequiredBytes);

	// if splitting, add the free block to start of the list, then return the correct addr.
	if(splitAddr != NULL){ // if split is needed
		debug("3-1. If allocation divides the block into two, add the second block to freelist(splitAddr=%p)\n", splitAddr);

		// set split block
		fillHeader(splitAddr, 0, (numOfNewBytes - numOfRequiredBytes) >> 4, 0, 0);
		fillFooter(splitAddr, 0, (numOfNewBytes - numOfRequiredBytes) >> 4);

		addToHead(splitAddr);

		numOfBlockBytes = numOfRequiredBytes;
	} else { // if no split is needed
		debug("3-2. If allocation does not divide the existing block\n");
		numOfBlockBytes = now->header.block_size << 4;
	}
	
	fillHeader(now, 1, numOfBlockBytes >> 4, paddingSize, 0);
	fillFooter(now, 1, numOfBlockBytes >> 4);

	debug("4. Return the correct address sf_sbrk_call=%d\n", sf_sbrk_call);
	debug("=================================================================================\n");
	return (void*)(((char*)now) + SF_HEADER_SIZE);
}

/*sf_free*/
void sf_free(void *ptr){
	bool validPointerToReturn = validatePointer(ptr);
	if(validPointerToReturn == 0){ // pointer is invalide

	} else{
		frees += 1;
	}

	sf_free_internal(ptr);
}

void sf_free_internal(void *ptr){ // input is the address sf_malloc returned, not start address of header
	bool validPointerToReturn;
	int coalesceType;
	sf_header *header;
	sf_free_header *free_header, *beforeBlockHeader, *afterBlockHeader;
	sf_footer *footer, *beforeBlockFooter, *afterBlockFooter;

	header = NULL;
	beforeBlockHeader = NULL;
	afterBlockHeader = NULL;
	footer = NULL;
	debug("=================================================================================\n");
	debug("1. Validate Argument, i.e., check if the ptr is actual pointer that has been assigned\n");

	validPointerToReturn = validatePointer(ptr);

	if(validPointerToReturn == 0){ // pointer is invalide
		debug("1-1. If it is invalid, return and do nothing\n");
		errno = EINVAL;
		return;
	}
	else{
		debug("1-2. If it is a valid, free the block\n");

		// first find the address of header and footer
		// Note. it is needed for later code!
		header = (sf_header *)((char*) ptr - SF_HEADER_SIZE);
		footer = (sf_footer *)((char*) header + (header->block_size << 4) - SF_FOOTER_SIZE);

		header->alloc = 0;
		fillHeader(header, 0, header->block_size, 0, 0);// Note. block_size is not changed
		fillFooter(header, 0, header->block_size);// Note. block_size is not changed

		free_header = (sf_free_header*)header;
	}
	
	coalesceType = checkCoalesce(ptr, &beforeBlockHeader, &afterBlockHeader, &beforeBlockFooter, &afterBlockFooter);
	debug("2. Check for possible coalescing (coalesceType=%d)\n", coalesceType);

	if(coalesceType == 0){
		// do nothing
	} else if(coalesceType == 1){

		// set the header block_size to proper value
		free_header->header.block_size += afterBlockHeader->header.block_size;

		// set the footer block_size the same
		afterBlockFooter->block_size = free_header->header.block_size;

		// remove the free block node from the list
		removeNodeFromDoublyLinkedList(afterBlockHeader);

		// set the new head as header
		// header = header; // No need to do this
	} else if(coalesceType == 2){ // return 2, if block before is free but not block after is alloced
		// set the header block_size to proper value
		beforeBlockHeader->header.block_size += free_header->header.block_size;

		// set the footer block_size the same
		footer->block_size = beforeBlockHeader->header.block_size;

		// sf_snapshot(true);
		// remove the free block node from the list
		removeNodeFromDoublyLinkedList(beforeBlockHeader); // don't have to because it is not in the list anyways

		// set the new head as header
		free_header = beforeBlockHeader;

	} else if(coalesceType == 3){
		// set the header block_size to proper value
		beforeBlockHeader->header.block_size += (free_header->header.block_size + afterBlockHeader->header.block_size);

		// set the footer block_size the same
		afterBlockFooter->block_size = beforeBlockHeader->header.block_size;

		// remove the free block node from the list
		removeNodeFromDoublyLinkedList(beforeBlockHeader); 
		removeNodeFromDoublyLinkedList(afterBlockHeader);

		// set the new head as header
		free_header = beforeBlockHeader;

	} else{
		error("coalesceType failed! coalesceType=%d\n", coalesceType);
		return;
	}

	debug("2-2. Add to the front of the freelist\n");
	addToHead(free_header);

	debug("=================================================================================\n");
}

/*sf_realloc*/
// The realloc() function changes the size of the memory block pointed to by ptr to size bytes.
// The contents will be unchanged in the range from the start of the region up to the minimum of the old and new sizes.
// If the new size is larger than the old size, the added memory will not be initialized.
// If ptr is NULL, then the call is equivalent to malloc(size), for all values of size;
// If size is equal to zero, and ptr is not NULL, then the call is equivalent to free(ptr).
// Unless  ptr  is  NULL,  it  must  have  been returned by an earlier call to malloc(), calloc() or realloc().
// If the area pointed to was moved, a free(ptr) is done.

// 1. Validate Argument, i.e., check if the ptr is actual pointer that has been assigned, check if the size is valid
// 1-1. Check if ptr is NULL
// 		1-1-1. ptr is null -> return sf_malloc
// 		1-1-2. ptr is not null
// 1-2. Check if ptr is returned by an earlier call of malloc or realloc
// 1-2-1. Invalid pointer return NULL
// 1-2-2. Valid pointer continue realloc

// 2. Validate size Argument, i.e., check if the size is valid
// 2-1. Size == 0, go for sf_free() -> return NULL? //TODO
// 2-2. New mem > old mem
// 2-3. New mem < old mem
// 2-4. New mem = old mem

// 3. Return appropriate address
void *sf_realloc(void *ptr, size_t size){
	int isValidPtr;

	debug("=================================================================================\n");
	debug("1. Validate ptr(%p) Argument, i.e., check if the ptr is actual pointer that has been assigned\n", ptr);

	debug("1-1. Check if ptr is NULL\n");
	if(ptr == NULL){
		debug("1-1-1. ptr is null -> return sf_malloc\n");
		return sf_malloc_internal(size);
	}
	else {
		debug("1-1-2. ptr is not null\n");
		if( ((sf_header*)( (char*)ptr - 8 ))->alloc == 0 ) {
			error("Passed free block... returning NULL\n");
			errno = EINVAL;
			return NULL;
		}
	}

	debug("1-2. Check if ptr is returned by an earlier call of malloc or realloc\n");
	isValidPtr = validatePointer(ptr); // return 0 if invalid
	if(isValidPtr == 0){
		debug("1-2-1. Invalid pointer return NULL\n");
		errno = EINVAL;
		return NULL;
	} else{
		debug("1-2-2. Valid pointer continue realloc\n");
	}

	debug("2. Validate size(%lu) Argument, i.e., check if the size is valid\n", size);
	if(size == 0){
		debug("2-1. Size == 0 & ptr != NULL, go for sf_free()\n");
		errno = EINVAL;
		return NULL;
	}

	sf_header *header = (sf_header*)( (char*)ptr - SF_HEADER_SIZE );
	size_t oldSize = (header->block_size << 4) - 16;
	int paddingSize = calculatePaddingSize(size);
	int numOfNewBytes = (header->block_size << 4);
	int numOfRequiredBytes = size + paddingSize + SF_HEADER_SIZE + SF_FOOTER_SIZE;
	
	debug("Trying to allocate block_size(=%d)(pad=%d)\n", numOfRequiredBytes, paddingSize);
	if(size > oldSize){
		debug("2-2. New mem size > old mem size(=%lu)\n", oldSize);
		
		sf_header *aHead = (sf_header*)( (char*)header + (header->block_size << 4) );

		// check if the next block is freed and together will be enough for realloc, without creating splinter
		if(aHead->alloc == 0 && (aHead->block_size << 4) + (header->block_size << 4) >= numOfRequiredBytes){
			void *splitAddr = needSplit((sf_free_header*)header, numOfNewBytes + (aHead->block_size << 4), numOfRequiredBytes); // sf_free_header, numOfNewBytes, numOfRequiredBytes

			if(splitAddr != NULL){ // no splinter is created
				debug("2-2-1. Coalescing with adjacent free block would be enough without creating splinter(splitAddr=%p)\n", splitAddr);

				// First remove the adjacent free block from the doubly linked list
				removeNodeFromDoublyLinkedList( (sf_free_header*)aHead);

				// Second, set the header and footer
				fillHeader(header, 1, numOfRequiredBytes >> 4, paddingSize, 0);
				fillFooter(header, 1, numOfRequiredBytes >> 4);

				// Third, adjust the next block and add to freelist
				fillHeader(splitAddr, 0, (numOfNewBytes + (aHead->block_size << 4) - numOfRequiredBytes) >> 4, 0, 0);
				fillFooter(splitAddr, 0, (numOfNewBytes + (aHead->block_size << 4) - numOfRequiredBytes) >> 4);

				addToHead(splitAddr);

				return ptr;
			}
			else { // check if the next block is freed and together will be enough for realloc, with creating splinter, so avoid it
				debug("2-2-2. Coalescing with adjacent free block would be enough, with avoiding splinter creation\n");

				// First remove the adjacent free block from the doubly linked list
				removeNodeFromDoublyLinkedList( (sf_free_header*)aHead);

				// Second, set the header and footer
				fillHeader(header, 1, (numOfNewBytes + (aHead->block_size << 4)) >> 4, paddingSize, 0);
				fillFooter(header, 1, (numOfNewBytes + (aHead->block_size << 4)) >> 4);

				// Note. no new free header is created

				return ptr;
			}
		}
		// either the next block is allocated or not enough for the size
		else {
			debug("2-2-3. Coalescing is not enough, calling sf_malloc internally...\n");

			int alreadyFreed = 0;

			if(aHead->alloc == 0){
				alreadyFreed = 1;
				sf_free_internal(ptr);
			}

			void *mallocRet = sf_malloc_internal(size);
			if(mallocRet == NULL){
				debug("sf_malloc(size=%lu) failed\n", size);
			}
			else {
				debug("sf_malloc(size=%lu) success\n", size);

				debug("sf_free(%p)\n", ptr);
				if(alreadyFreed == 0){
					sf_free_internal(ptr); // free the returned
				}
			}

			return mallocRet;
		}
	}
	else if (size < oldSize){
		debug("2-3. New mem size (=%lu) < old mem size(=%lu)\n", size, oldSize);
		void *splitAddr = needSplit((sf_free_header*)header, numOfNewBytes, numOfRequiredBytes); // sf_free_header, numOfNewBytes, numOfRequiredBytes

		// if(oldSize - size < 32) { // if shrinking causes a splinter
		if(splitAddr == NULL){ // Same meaning as above line
			debug("2-3-1. oldSize(%lu) - size(%lu) < 32\n", oldSize, size);
			// do nothing and return
			return ptr;
		} else{
			debug("2-3-2. oldSize(%lu) - size(%lu) >= 32\n", oldSize, size);
			// split the block
			fillHeader(ptr, 1, numOfRequiredBytes >> 4, paddingSize, 1); // 1 = isAlloc, 1 = isPayloadAddr
			fillFooter( (sf_header*)((char*)ptr - 8), 1, numOfRequiredBytes >> 4);
			// sf_varprint(ptr);

			fillHeader(splitAddr, 0, (numOfNewBytes - numOfRequiredBytes) >> 4, 0, 0);
			fillFooter(splitAddr, 0, (numOfNewBytes - numOfRequiredBytes) >> 4);

			// now add splitAddr to the front of the doubly linked list
			addToHead(splitAddr);

			sf_free_header *free_header, *beforeBlockHeader, *afterBlockHeader;
			sf_footer *footer, *beforeBlockFooter, *afterBlockFooter;

			header = (sf_header *)((char*)splitAddr);
			beforeBlockHeader = NULL;
			afterBlockHeader = NULL;
			footer = (sf_footer *)((char*)header + (numOfNewBytes - numOfRequiredBytes - SF_FOOTER_SIZE));

			free_header = (sf_free_header *)header;

			int coalesceType = checkCoalesce( (char*)splitAddr + 8, &beforeBlockHeader, &afterBlockHeader, &beforeBlockFooter, &afterBlockFooter);
			debug("2-3-3. Check for possible coalescing (coalesceType=%d)\n", coalesceType);

			if(coalesceType == 0){
				// do nothing
			} else if(coalesceType == 1){
				// set the header block_size to proper value
				free_header->header.block_size += afterBlockHeader->header.block_size;

				// set the footer block_size the same
				afterBlockFooter->block_size = free_header->header.block_size;

				// remove the free block node from the list
				removeNodeFromDoublyLinkedList(afterBlockHeader);

				// set the new head as header
				// header = header; // No need to do this
			} else if(coalesceType == 2){ // return 2, if block before is free but not block after is alloced
				// set the header block_size to proper value
				beforeBlockHeader->header.block_size += free_header->header.block_size;

				// set the footer block_size the same
				footer->block_size = beforeBlockHeader->header.block_size;

				// sf_snapshot(true);
				// remove the free block node from the list
				removeNodeFromDoublyLinkedList(beforeBlockHeader); // don't have to because it is not in the list anyways

				// set the new head as header
				free_header = beforeBlockHeader;

			} else if(coalesceType == 3){
				// set the header block_size to proper value
				beforeBlockHeader->header.block_size += (free_header->header.block_size + afterBlockHeader->header.block_size);

				// set the footer block_size the same
				afterBlockFooter->block_size = beforeBlockHeader->header.block_size;

				// remove the free block node from the list
				removeNodeFromDoublyLinkedList(beforeBlockHeader); 
				removeNodeFromDoublyLinkedList(afterBlockHeader);

				// set the new head as header
				free_header = beforeBlockHeader;

			} else{
				error("coalesceType failed! coalesceType=%d\n", coalesceType);
			}

			debug("2-3-4. Add to the front of the freelist\n");
			// addToHead(free_header);

			// return the address
			return ptr;
		}
	}
	else {
		debug("2-4. New mem = old mem size(=%lu)\n", oldSize);

		// do nothing and return
		return ptr;
	}
}

int sf_info(info* meminfo){
	if(meminfo == NULL){
		return -1;
	}

	internal = 0;
	sf_header *alloc_now = (sf_header*)((char*)sf_sbrk(0) - 4096 * sf_sbrk_call);
	while( (void*)alloc_now < sf_sbrk(0)){
		if(alloc_now->alloc == 0){
			alloc_now = (sf_header*)((char*)alloc_now + (alloc_now->block_size << 4));
			continue;
		}

		internal += (16 + alloc_now->padding_size);
		alloc_now = (sf_header*)((char*)alloc_now + (alloc_now->block_size << 4));
	}

	external = 0;
	sf_free_header *free_now = freelist_head;
	while( (void*)free_now < sf_sbrk(0)){
		if(free_now == NULL){
			break;
		}

		external += (free_now->header.block_size << 4);
		free_now = free_now->next;
	}

	meminfo->internal = internal;
	meminfo->external = external;
	meminfo->allocations = allocations;
	meminfo->frees = frees;
	meminfo->coalesce = coalesce;

	return 0;
}
