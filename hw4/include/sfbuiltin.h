#ifndef _SFBUILTIN_H_
#define _SFBUILTIN_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#define COLOR
#ifdef COLOR
    #define NOML  "\x1B[0"
    #define BOLD  "\x1B[1"

    #define KNRM  "m"

    #define KBLK  ";30m"
    #define KRED  ";31m"
    #define KGRN  ";32m"
    #define KYEL  ";33m"
    #define KBLU  ";34m"
    #define KMAG  ";35m"
    #define KCYN  ";36m"
    #define KWHT  ";37m"
#else
    /* Color was either not defined or Terminal did not support */
    #define NOML
    #define BOLD

    #define KNRM
    #define KBLK
    #define KRED
    #define KGRN
    #define KYEL
    #define KBLU
    #define KMAG
    #define KCYN
    #define KWHT
#endif

#ifdef DEBUG
    #define KNRM2  "\x1B[0m"
    #define KRED2  "\x1B[1;31m"
    #define KGRN2  "\x1B[1;32m"
    #define KYEL2  "\x1B[1;33m"
    #define KBLU2  "\x1B[1;34m"
    #define KMAG2  "\x1B[1;35m"
    #define KCYN2  "\x1B[1;36m"
    #define KWHT2  "\x1B[1;37m"
    #define KBWN2  "\x1B[0;33m"

    #define debug(S, ...)   fprintf(stderr, KMAG2 "DEBUG: %s:%s:%d " KNRM2 S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define error(S, ...)   fprintf(stderr, KRED2 "ERROR: %s:%s:%d " KNRM2 S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL2 "WARN: %s:%s:%d " KNRM2 S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU2 "INFO: %s:%s:%d " KNRM2 S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN2 "SUCCESS: %s:%s:%d " KNRM2 S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
    #define debug(S, ...)
    #define error(S, ...)   fprintf(stderr, KRED2 "ERROR: " KNRM2 S, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL2 "WARN: " KNRM2 S, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU2 "INFO: " KNRM2 S, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN2 "SUCCESS: " KNRM2 S, ##__VA_ARGS__)
#endif

char cd_history[PATH_MAX];
struct INT_STR{
	int val;
} last_exe;
char *HOME_DIR;

int PROMPT_USER, PROMPT_HOST;
char *PROMPT_COLOR_USER, *PROMPT_COLOR_HOST, *PROMPT_BOLD_USER, *PROMPT_BOLD_HOST;

int preprocess();
int builtin_help();
int builtin_exit();
int builtin_cd(char** argv);
int builtin_pwd();
int builtin_prt();
int builtin_chpmt(char** argv);
int builtin_chclr(char** argv);


#endif