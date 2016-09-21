#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

/* for stderror */
#include <string.h>
/* for errno*/
#include <errno.h>
/* for file size */
#include <sys/stat.h>
#include <sys/types.h>
/* for fullpath*/
#include <limits.h>
#include <dirent.h>
/* for hostname*/
#include <netdb.h>
/* for uname*/
#include <sys/utsname.h>
#include <rpc/rpc.h>
/* for timing*/
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>


/** The enum for endianness. */
typedef enum {NO_BOM, LITTLE, BIG} endianness;

endianness checkBom(int fd);
void writeBom(int fd, endianness end);

#endif