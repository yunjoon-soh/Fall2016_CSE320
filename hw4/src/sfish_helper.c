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

	buf[tokCnt] = 0;

	return buf;
}

int countElements(char* cmd){
	int count = 0;
	int flag = 1; // initially, assume that it has space before the whole cmd

	char *c = cmd;
	while( *c != '\0'){ // while cmd has not reached the end
		if( *c == ' ' ){ // if it is space, flag that it has encountered a space
			flag = 1;
		} else if( *c == '|' || *c == '<' || *c == '>'){
			count++;
			flag = 1; // what comes after has to be counted separatedly from this
		} else{ // just normal chars
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

// int getNextPipe(char** argv, int from){
// }

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

int isPath(char* pwd){
	while(*pwd != '\0'){
		if(*pwd == '/'){
			return !SF_FALSE;
		}
		pwd++;
	}

	return SF_FALSE;
}

int existsInPath(char* cmd){
	int ret;
	char *env = getenv("PATH");
	char *start = env, *end;
	char buf[PATH_MAX];
	struct stat statBuf;

	while( (end = strchr(start, ':')) != NULL){
		*end = '\0';
		strcpy(buf, start);
		*end = ':';
		strcat(buf, "/");
		strcat(buf, cmd);
		// printf("Looking for %s in %s\n", cmd, buf);
		
		ret = stat(buf, &statBuf);
		if(ret == 0){
			env = getenv("PATH");
			// printf("Found!\n%s\n", env);
			
			return !SF_FALSE;
		} else{
			start = end + 1;
		}
	}

	return SF_FALSE;
}

int exeCmd(int argc, char** argv, char* envp[]){
	int ret;
	char *env = getenv("PATH");
	printf("%s\n", env);

	
	if(isPath(argv[0]) == SF_FALSE){
		// not a path
		if(existsInPath(argv[0]) != SF_FALSE){
			ret = execvp(argv[0], argv);
			if(ret == -1){
				//TODO
				fprintf(stderr, "Failed to execute %s\n", argv[0]);
				for(int i = 0; i < 3; i++){
					printf("argv[%d]=%s\n", i, argv[i]);
				}
			}
			printf("*isPath:False, Ending\n");
			exit(0);
		} else {
			fprintf(stderr, "No such command!\n");
			exit(1);
		}		
	} else {
		printf("*isPath:True\n");
		// is path
		ret = execv((const char*)argv[0], argv);
		if(ret == -1){
			//TODO
		}
		printf("*isPath:True, Ending\n");
		exit(0);
	}

    return 0;
}

int isBgProc(char* cmd){
    return SF_FALSE;
}

// int pipelineCheck(int argc, char** argv){
//     next_pipe = 0;
//     while( (next_pipe = getNextPipe(argv, next_pipe)) != -1 ){ 
//         char *filename = getFileNameFromPipeArg(argv, next_pipe);
        
//         // next_pipe is -1 when there is no more pipe exists
//         if( (strstr_ret = strstr(argv[next_pipe], "|")) == 0 ){
//             int open_fd = Open(filename, flags);
            
//         } else if( (strstr_ret = strstr(argv[next_pipe], ">")) == 0 ){
            
//         } else if( (strstr_ret = strstr(argv[next_pipe], "<")) == 0 ){
            
//         } else {
//             error("This code reached because I implemented getNextPipe(..) incorrectly\n");
//             break;
//         }
//     }
// }