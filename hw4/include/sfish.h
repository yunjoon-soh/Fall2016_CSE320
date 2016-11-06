#ifndef _SFISH_H_
#define _SFISH_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <time.h>

#include "sfish_helper.h"
#include "sfbuiltin.h"
#include "sfconst.h"
#include "job_arraylist.h"

struct exe_info{
	int prog_index;
	int pipe_fd[2];
	int stderr_pipe;
};

int Fork(int argc, char** argv, char *envp[], struct exe_info* ei);
int Fork_Builtin(int pipe_fd[2], int stderr, int argc, char** argv, int prog);
int Fork_Cmd(int pipe_fd[2], int stderr, int argc, char** argv, char *envp[], int prog);



#endif // header guard