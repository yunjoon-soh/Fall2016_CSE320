#ifndef _SFISH_H_
#define _SFISH_H_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "sfish_helper.h"
#include "sfbuiltin.h"
#include "sfconst.h"

int Fork(int pipe_fd[2], int argc, char** argv, char* envp[], int prog);
int Fork_Builtin(int pipe_fd[2], int argc, char** argv, int prog);
int Fork_Cmd(int pipe_fd[2], int argc, char** argv, char *envp[], int prog);

#endif // header guard