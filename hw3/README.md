# Homework 3 - Memory Allocator

## Yun Joon (Daniel) Soh (108256259)
* Used One Grace day
* Added a lot of unit tests (about 42 additional tests)

## errno
* sf_malloc(), sf_free(), sf_realloc() sets errno to EINVAL, if arguments are invalid.
* When other errno is set, it is from internal calls to other funcs not implemented by the author.
e.g., errno == ENOMEM when asking for page beyond 4 pages. (Done by sf_sbrk());

## sfunit.c
* Some tests may intentionally send out errors. (This is intential)
* MUST INCLUDE <time.h> and <errno.h> #include <time.h> #include <errno.h>
Note. <time.h> is used for randomized testing.
* Implemented personal wrapper functions to ease the coding. (e.g., checkHeaderAndFooter, etc.)
Note. It is included in sfunit.c after 
/*
//############################################
// STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
// DO NOT DELETE THESE COMMENTS
//############################################
*/
comment.

## sf_info detail
* internal : calculate just the header and padding sizes of allocated blocks
Note. it is impossible to keep track of otherwise-would-have-been-splinter but merged into block.
Reasoning. It is possible to keep track when sf_malloc() (i.e., block_size - size - SF_HEADER_SIZE - SF_FOOTER_SIZE). However, when such block is freed, there is no way to determine whether this block includes otherwise-would-have-been-splinter or not, other than keeping track of the acutal size that the user used to call sf_malloc(), which is not the intention of this homework.
* external : calcaulate block_size of free blocks in freelist.
* allocations : count successful sf_malloc() only!!!
* frees : count successful sf_free() only!!!
Note. there were a lot of discussions on counting allocations, frees when sf_realloc() but it does not converge (i.e., some not answered, some conflicts from previous answers, etc.)
* coalesce : count coalescing in all three funcs(sf_malloc, sf_free, sf_realloc).

## Tested with criterion libraries

Criterion - unit testing library
The MIT License (MIT)
Copyright © 2015-2016 Franklin "Snaipe" Mathieu <http://snai.pe/>