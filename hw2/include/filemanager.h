#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_

#include "main.h"

/* All returns -1 on failure*/
int openToRead(char* filename);
int openToWrite(char* filename, endianness target);
int openToCreate(char* filename);
int openToAppend(char* filename);

#define NO_FD -1

#endif