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
#include <time.h>

#include "sfconst.h"

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
	struct tm timeinfo;
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
void p_sigint_handler(int sig);

void sigtstp_handler(int sig);
void sigcont_handler(int sig);
void p_sigchld_handler(int sig);

#endif