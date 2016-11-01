#ifndef _SFISH_HELPER_H_
#define _SFISH_HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "sfconst.h"
#include "sfbuiltin.h"

// deliminate by ' ', '|', '<', '>'
// len is num of cmds, returned by countElements
char **parseNCmd(char* cmd, char* buf[], int len); 
int countElements(char* cmd);


char *getsnPrompt(char* buf, int len);

int exeBuiltIn(int argc, char** argv);
int exeCmd(int argc, char** argv, char* envp[]);

int isBgProc(char* cmd);

int Open(char* filename, int flags);

/* Pipeline*/
// returns character to the starting position of pipeline name
int pipelineCheck(int argc, char** argv);
char *getFileNameFromPipeArg(char** argv, int next_pipe);
int getNextPipe(char** argv, int from);

#endif 