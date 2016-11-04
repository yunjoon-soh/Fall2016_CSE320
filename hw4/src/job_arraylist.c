//job_arraylist.c

#include "job_arraylist.h"

int job_cnt = 0;

int addJob(struct job* newJob){
	if (job_start == NULL && job_end == NULL){
		// empty list
		job_start = newJob;
		job_end = newJob;
	} else{
		job_end->next = newJob;
		newJob->prev = job_end;

		job_end = newJob;
	}

	return JOB_TRUE;
}

// return NULL on failure
struct job* createJob(int pid, job_state jstate, char** cmd){
	struct job* newJob = (struct job*) malloc(sizeof(struct job));
	if(newJob == NULL){
		fprintf(stderr, "Malloc for newJob failed\n");
		return NULL;
	}

	int len = 1; // for the last '\0'
	char **cmd_ptr = cmd;
	while( *cmd_ptr != 0 ){
		len += strlen(*cmd);
		cmd_ptr++;
	}

	newJob->jid = ++job_cnt;
	newJob->pid = pid;
	newJob->jstate = jstate;
	newJob->cmd = (char*) malloc(sizeof(char) * len);
	if(newJob->cmd == NULL){
		fprintf(stderr, "Malloc for newJob->cmd failed\n");
		return NULL;
	}
	// copy cmd one by one
	char *cpyLoc = newJob->cmd;
	cmd_ptr = cmd;
	while( *cmd_ptr != 0 ){
		len = strlen(*cmd_ptr);
		strncpy(cpyLoc, *cmd_ptr, len);
		cmd_ptr++;
		cpyLoc += len;

		if(*cmd_ptr != 0){
			*cpyLoc = ' ';
			cpyLoc += 1;
		} else{
			*cpyLoc = '\0';
		}
	}

	newJob->prev = NULL;
	newJob->next = NULL;

	return newJob;
}

int removeJob(int jid_pid, int isjid){
	struct job *now = job_start;

	if(now == NULL){
		// empty list
		return JOB_FALSE;
	}

	while( equals(now, jid_pid, isjid) != JOB_TRUE ){
		now = now->next;
		if(now == NULL){ // reached the end of the list
			return JOB_FALSE;
		}
	}

	fprintf(stderr, "about to delete now=%d\n", now->pid);
	if(now->prev == NULL && now->next == NULL){
		fprintf(stderr, "just this one left\n");
		// now is headjob and the tailjob
		job_start = NULL;
		job_end = NULL;
	} else if(now->prev == NULL && now->next != NULL){
		fprintf(stderr, "rm head\n");
		// head job but not tailjob
		now->next->prev = NULL;
		job_start = now->next;
	} else if(now->prev != NULL && now->next == NULL){
		fprintf(stderr, "rm tail\n");
		// tail job but not headjob
		now->prev->next = NULL;
		job_end = now->prev;
	} else {
		fprintf(stderr, "rm middle\n");
		// neither head nor tail
		now->prev->next = now->next;
		now->next->prev = now->prev;
	}

	free(now->cmd);
	free(now);

	return JOB_TRUE;
}

void printJobs(){
	struct job *now = job_start;
	fprintf (stdout, "Current job_start=%d, job_end=%d\n", (now==NULL)?0:now->pid, (job_end==NULL)?0:job_end->pid);
	while(now != NULL){
		// fprintf(stdout, "[%d]     %s       %5d     \"%s\"\n", now->jid, 
			// (now->jstate == RUNNING)?"Running":"Stopped", now->pid, now->cmd);
		int prev_pid, next_pid;
		if(now->prev != NULL){
			prev_pid = now->prev->pid;
		} else{
			prev_pid = 0;
		}
		if(now->next != NULL){
			next_pid = now->next->pid;
		} else{
			next_pid = 0;
		}
		fprintf(stdout, "[%d]     %s       %5d     \"%s\" prev's pid=%d, next's pid=%d\n", now->jid, 
			(now->jstate == RUNNING)?"Running":"Stopped", now->pid, now->cmd, prev_pid, next_pid);
		now = now->next;
	}
}

// return JOB_TRUE, JOB_FALSE
int equals(struct job* j, int jid_pid, int isjid){
	if(isjid == JOB_TRUE){
		if(j->jid == jid_pid){
			return JOB_TRUE;
		} else{
			return JOB_FALSE;
		}
	} else {
		if(j->pid == jid_pid){
			return JOB_TRUE;
		} else{
			return JOB_FALSE;
		}
	}
}