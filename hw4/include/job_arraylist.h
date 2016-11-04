#ifndef _JOB_ARRAYLIST_H_
#define _JOB_ARRAYLIST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "sfconst.h"

#ifndef COLOR
#define COLOR
#endif

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


typedef enum {RUNNING, STOPPED} job_state;
struct job *job_start, *job_end, *fg;
struct job{
	// don't change after first creation
	int pid;
	char* cmd;
	// change depending on context
	int jid;
	job_state jstate;
	volatile sig_atomic_t inJob;

	struct job* prev;
	struct job* next;
};

struct job* createJob(int pid, job_state jstate, char** cmd);

int addJob(struct job* newJob);

int removeJob(int jid_pid, int isjid);
void removeAllJobs();

void printJobs();
void printJobsAll(); // print every child processes

int equals(struct job* j, int jid_pid, int isjid);
struct job* findById(int jid_pid, int isjid);

void p_sigtstp_handler(int sig);
void sigtstp_handler(int sig);
void sigcont_handler(int sig);

#endif