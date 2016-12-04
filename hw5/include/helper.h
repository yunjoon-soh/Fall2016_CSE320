// helper.h
#ifndef _HELPER_H_
#define _HELPER_H_

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <dirent.h>
#include <semaphore.h>

#include "list.h"
#include "map_reduce.h"

#define BUF_SIZE 10

// Colors
#ifndef COLOR
	#define COLOR
	#define KNRM  "\x1B[0m"
	#define KRED  "\x1B[1;31m"
	#define KGRN  "\x1B[1;32m"
	#define KYEL  "\x1B[1;33m"
	#define KBLU  "\x1B[1;34m"
	#define KMAG  "\x1B[1;35m"
	#define KCYN  "\x1B[1;36m"
	#define KWHT  "\x1B[1;37m"
	#define KBWN  "\x1B[0;33m"
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

// funcs
char **splitByComma(char *line, char *buf[], size_t cnt);
int cntry_code_converter(char code[2]);
char** cntry_code_reverter(int code, char **buf);

// wrapper
FILE **Fopen(const char *path, const char *mode, FILE **fp);
DIR **Opendir(const char *name, DIR **dir);
struct dirent *Readdir(DIR *dirp, struct dirent **ent);
int Closedir(DIR **pdir);
void Fread_r(struct map_res **res, FILE *fp);
void Fwrite_r(struct map_res *res, FILE *fp);

int Sem_init(sem_t *sem, int pshared, unsigned int value);
int P(sem_t* sem);
int V(sem_t* sem);

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int Pthread_setname(pthread_t thread, const char *name);
int Pthread_join(pthread_t thread, void **retval);

extern sem_t mutex, w, line;
extern sem_t slots, items; // part 4
extern size_t readcnt, linecnt;
extern size_t start, end;
extern struct map_res *buf[];

void Read_struct_r(struct map_res **res);
void write_to_buf(struct map_res* buf, struct map_res *res);
void Write_struct_r(struct map_res *res);

#endif