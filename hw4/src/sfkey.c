/*sfkey.c*/
#include "sfkey.h"
#include "job_arraylist.h"
#include "sfbuiltin.h"

volatile sig_atomic_t SPID = -1;
extern int cmd_cnt;

#define SFISH_INFO_LENGTH 16
const char* SFISH_INFO[SFISH_INFO_LENGTH] = {
	"----Info----",
	"help",
	"prt",
	"----CTRL----",
	"cd",
	"chclr",
	"chpmt",
	"pwd",
	"exit",
	"----Job Control----",
	"bg",
	"fg",
	"disown",
	"jobs",
	"kill",
	"----Number of Commands Run----"
};


void ctrl_b(){
	struct job *now = job_start;
	if(now == NULL){
		SPID = -1;
	} else {
		while(now->inJob == 0){
			now = now->next;
			if(now == NULL){
				break;
			}
		}
	}

	SPID = (now==NULL)?-1:now->pid;
	fprintf(stdout, "SPID is %d\n", SPID);
}

void ctrl_g(){
	if(SPID == -1){
		fprintf(stderr, "SPID does not exists and has been set to -1\n");
		return;
	}

	struct job *found = findById(SPID, JOB_FALSE);
	if(found == NULL){
		fprintf(stderr, "SPID does not exists and has been set to -1\n");
		SPID = -1;
		return;	
	}

	if(found->inJob == 1){ // if bg process
		sigset_t mask, prev_mask;

		sigemptyset(&mask);
		sigaddset(&mask, SIGCHLD);

	    sigprocmask(SIG_BLOCK, &mask, &prev_mask);
		kill(found->pid, SIGTERM);
		kill(found->pid, SIGCONT);
		sigprocmask(SIG_SETMASK, &prev_mask, NULL);

		fprintf(stdout, "[%d] %d stopped by signal %d\n", found->jid, found->pid, SIGTERM);
	}
}

void ctrl_h(){
	builtin_help();
}

void ctrl_p(){
	for(int i = 0; i < SFISH_INFO_LENGTH; i++){
		fprintf(stdout, "%s\n", SFISH_INFO[i]);
	}

	fprintf(stdout, "%d\n", cmd_cnt);
	fprintf(stdout, "----Process Table----\n");
	fprintf(stdout, "%7s %7s %7s %10s\n", "PGID", "PID", "TIME", "CMD");

	struct job *now = job_start;
	if(now == NULL){
		return;
	} else {
		while(now != NULL){
			if(now->jstate == RUNNING){
				fprintf(stdout, "%7d %7d    %2d:%2d %10s\n", getpgid(now->pid), now->pid, 
					now->timeinfo.tm_hour, now->timeinfo.tm_min, now->cmd);
			}
			now = now->next;
		}
	}
}