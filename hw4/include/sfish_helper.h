#ifndef _SFISH_HELPER_H_
#define _SFISH_HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "sfconst.h"
#include "sfbuiltin.h"

char **parseNCmd(char* cmd, char* buf[], int len); // len is num of cmds
int countElements(char* cmd);
char *getsnPrompt(char* buf, int len);

int exeBuiltIn(int argc, char** argv);
int exeCmd(char** argv);

// bool isBuiltIn(char* cmd);
// deprecated: instead used exeBuiltIn(char** cmds)

int isBgProc(char* cmd);

#endif