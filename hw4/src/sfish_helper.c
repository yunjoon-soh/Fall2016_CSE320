/** sfish_helper.c
*/
#include "sfish_helper.h"

// return NULL on failure
char **parseNCmd(char* cmd, char* buf[], int len){
	int tokCnt = 0; // number of tokens found so far
	char *c = cmd;

	if(strlen(cmd) <= 0){
		buf[0] = c;
		return buf;
	}

	// first token
	buf[tokCnt++] = c;
	while(*c != ' ' && *c != '\0'){
		if(*c == '\0'){ break; }
		c++;
	}
	if(*c == '\0'){ return buf; }
	*c = '\0'; // ' ' -> '\0', i.e., split the token just found

	while(1){
		buf[tokCnt++] = (++c); // next token is found

		while(*c != ' ' && *c != '\0'){
			if(*c == '\0'){ break; }
			c++;
		}
		if(*c == '\0') { break; }

		*c = '\0'; // ' ' -> '\0'

		if(tokCnt >= len){ // if last intended token was found, quit
			fprintf(stderr, "Length of commands maxed out\n");
			break;
		}
	}

	return buf;
}

int countSpaces(char* cmd){
	int count = 0;
	char *c = cmd;
	while( *(c = (c+1)) != '\0'){
		if( *c == ' ' )
			count++;
	}
	return count;
}

char *getsnPrompt(char* buf, int len){
    char *prompt = "sfish";
    char *user = getenv("USER");
    char cdir[PATH_MAX];
    
    char host[HOST_NAME_MAX];
    gethostname(host, HOST_NAME_MAX);

    char *pwd = getcwd(cdir, PATH_MAX);

    if( strcmp(pwd, HOME_DIR) == 0)
    	strcpy(cdir, "~");

    if(PROMPT_USER == PROMPT_ENABLED && PROMPT_HOST == PROMPT_ENABLED){
    	snprintf(buf, len, "%s-%s@%s:[%s]> ", prompt, user, host, pwd);
    } else if(PROMPT_USER == PROMPT_ENABLED && PROMPT_HOST != PROMPT_ENABLED){
    	snprintf(buf, len, "%s-%s:[%s]> ", prompt, user, pwd);
    } else if(PROMPT_USER != PROMPT_ENABLED && PROMPT_HOST == PROMPT_ENABLED){
    	snprintf(buf, len, "%s-%s:[%s]> ", prompt, host, pwd);
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
    }

    return SF_FAIL;
}

int exeCmd(char** cmds){
    return 0;
}

int isBgProc(char* cmd){
    return SF_FALSE;
}