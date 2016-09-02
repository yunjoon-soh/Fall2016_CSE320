#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

int validateargs(int argc, char** argv);
int isValidDir(char* dir);

int nfiles(char* dir);

int map(char* dir,void* results, size_t size, int (*act)( FILE * f, void * res, char * fn));