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
#include <fcntl.h>

#define JOB_TRUE 1
#define JOB_FALSE 0

typedef enum {RUNNING, STOPPED} job_state;
struct job *job_start, *job_end;
struct job{
	int jid;
	job_state jstate;
	int pid;
	char* cmd;
	struct job* prev;
	struct job* next;
};

int addJob(struct job* newJob);
struct job* createJob(int pid, job_state jstate, char** cmd);
int removeJob(int jid_pid, int isjid);
void printJobs();
int equals(struct job* j, int jid_pid, int isjid);

#endif