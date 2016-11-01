#ifndef _SFISH_HELPER_H_
#define _SFISH_HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "sfconst.h"
#include "sfbuiltin.h"

// deliminate by ' ', '|', '<', '>'
// len is num of cmds, returned by countElements
char **parseNCmd(char* cmd, char* buf[], int len); 
int countElements(char* cmd);

char *getsnPrompt(char* buf, int len);

int exeBuiltIn(int argc, char** argv);
int exeCmd(int argc, char** argv, char* envp[]);
int isBuiltin(char* argv_0);

int isBgProc(char* cmd);// int Open(char* filename, int flags);

// int Open(char* filename, int flags);

/* Pipeline*/
// returns character to the starting position of pipeline name
int pipelineCheck(int argc, char** argv);
int getNextPipe(int argc, char** argv, int from);
// char *getFileNameFromPipeArg(char** argv, int next_pipe);


#endif 