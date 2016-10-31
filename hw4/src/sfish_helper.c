/** sfish_helper.c
*/
#include "sfish_helper.h"

char **parseNCmd(char* cmd, char* buf[], int len){
	int tokCnt = 0; // number of tokens found so far
	char *c = cmd;

	while(*c == ' ' && *c != '\0'){// ignore all the whitespaces before
		c++;
	}

	if(strlen(c) <= 0 || len <= 0){
		return NULL;
	}

	while(1){
		while(*c == ' ' && *c != '\0'){// ignore all the whitespaces before
			c++;
		}
		if(*c == '\0') { break; } // last element found

		buf[tokCnt++] = c; // next token is found

		while(*c != ' ' && *c != '\0'){
			c++;
		}
		if(*c == '\0') { break; } // last element found

		*c++ = '\0'; // ' ' -> '\0'

		if(tokCnt > len){ // if last intended token was found, quit
			fprintf(stderr, "Length of commands maxed out\n");
			break;
		}
	}

	return buf;
}

int countElements(char* cmd){
	int count = 0;
	int flag = 1; // initially, assume that it has space before the whole cmd

	char *c = cmd;
	while( *c != '\0'){ // while cmd has not reached the end
		if( *c == ' ' ){ // if it is space flag that it has encountered a space
			flag = 1;
		} else{
			if(flag == 1){
				count++; // increament only when reaching a character right after white space
				flag = 0;
			}
		}

		c = (c+1);
	}
	return count;
}

char *getsnPrompt(char* buf, int len){
	int i;
    char *prompt = "sfish";
    char *user = getenv("USER");
    char cdir[PATH_MAX];
    
    char host[HOST_NAME_MAX];
    gethostname(host, HOST_NAME_MAX);

    char *pwd = getcwd(cdir, PATH_MAX);

    char flag = 0;
    char *ptr_pwd = pwd, *ptr_home = HOME_DIR;
    for(i = 0; i < strlen(HOME_DIR); i++){
    	if(*ptr_pwd++ != *ptr_home++) {
    		flag = 1;
    		break;
    	}
    }
    if(flag == 0){
    	strncpy(cdir, "~", 1);
    	strncpy(cdir+1, ptr_pwd, strlen(ptr_pwd) + 1);
    }

    // if( strncmp(pwd, HOME_DIR, strlen(HOME_DIR)) == 0)
    	// strcpy(cdir, "~");

    if(PROMPT_USER == PROMPT_ENABLED && PROMPT_HOST == PROMPT_ENABLED){
    	snprintf(buf, len, "%s-%s%s%s%s%s@%s%s%s%s%s:[%s]> ", prompt, 
    		PROMPT_BOLD_USER, PROMPT_COLOR_USER, user, NOML, KNRM,
    		PROMPT_BOLD_HOST, PROMPT_COLOR_HOST, host, NOML, KNRM,
    		pwd);
    } else if(PROMPT_USER == PROMPT_ENABLED && PROMPT_HOST != PROMPT_ENABLED){
    	snprintf(buf, len, "%s-%s%s%s%s%s:[%s]> ", prompt,
    		PROMPT_BOLD_USER, PROMPT_COLOR_USER, user, NOML, KNRM,
    		pwd);
    } else if(PROMPT_USER != PROMPT_ENABLED && PROMPT_HOST == PROMPT_ENABLED){
    	snprintf(buf, len, "%s-%s%s%s%s%s:[%s]> ", prompt, 
    		PROMPT_BOLD_HOST, PROMPT_COLOR_HOST, host, NOML, KNRM,
    		pwd);
    } else { // neither is enabled
    	snprintf(buf, len, "%s:[%s]> ", prompt, pwd);
    }

    return buf;
}

int exeBuiltIn(int argc, char** argv){
    char* cmd = argv[0];
    if( strcmp(cmd, "help") == 0){
        builtin_help();
        return SF_SUCCESS;
    } else if( strcmp(cmd, "exit") == 0){
        builtin_exit();
        return SF_SUCCESS;
    } else if( strcmp(cmd, "cd") == 0){
    	if(argc >= 2)
        	builtin_cd(argv[1]); // ignore the rest
    	else
    		builtin_cd(NULL);
        return SF_SUCCESS;
    } else if( strcmp(cmd, "pwd") == 0){
        builtin_pwd();
        return SF_SUCCESS;
    } else if( strcmp(cmd, "prt") == 0){
        builtin_prt();
        return SF_SUCCESS;
    } else if( strcmp(cmd, "chpmt") == 0){
    	builtin_chpmt(argc, argv);
    	return SF_SUCCESS;
    } else if( strcmp(cmd, "chclr") == 0){
    	builtin_chclr(argc, argv);
    	return SF_SUCCESS;
    }

    return SF_FAIL;
}

int exeCmd(char** cmds){
    return 0;
}

int isBgProc(char* cmd){
    return SF_FALSE;
}