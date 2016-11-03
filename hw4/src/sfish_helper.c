/** sfish_helper.c
*/
#include "sfish_helper.h"

char* LT_PIPE = "<";
char* GT_PIPE = ">";
char* JS_PIPE = "|"; // JS = JuSt
char* BG_RUN = "&";

char **parseNCmd(char** cmd, char* buf[], int len){
	int tokCnt = 0; // number of tokens found so far
	char *c = (*cmd);

	while(*c == ' ' && *c != '\0'){// ignore all the whitespaces before
		c++;
	}

	if(strlen(c) <= 0 || len <= 0){
		return NULL;
	}

	int flag = 0;
	while(*c != '\0'){
		if(*c == ' '){
			flag = 0;
			*c = '\0';
			c++;
			continue;
		}
		else { // c is not space
			if(*c == '<'){
				flag = 0;
				*c = '\0';
				buf[tokCnt++] = LT_PIPE;
			} else if(*c == '|'){
				flag = 0;
				*c = '\0';
				buf[tokCnt++] = JS_PIPE;
			} else if(*c == '>'){
				flag = 0;
				*c = '\0';
				buf[tokCnt++] = GT_PIPE;
			} else if(*c == '&'){
				flag = 0;
				*c = '\0';
				buf[tokCnt++] = BG_RUN;
			} else if(flag == 0){ // have been white space so far
				flag = 1;
				buf[tokCnt++] = c;
			} else if(flag == 1){
				// do nothing
			}
		}

		c++;
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
		} else if( *c == '|' || *c == '<' || *c == '>' || *c == '&'){
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

int exeBuiltIn(int argc, char** argv){
	char* cmd = argv[0];

	debug("exe builtin: cmd=%s\n", cmd);
	for(int i = 0; i < argc; i++){
        debug("argc=%d, argv[%d]=%s\n", argc, i, argv[i]);
    }

    if( strcmp(cmd, "help") == 0){
        return builtin_help();
    } else if( strcmp(cmd, "exit") == 0){
        return builtin_exit();
    } else if( strcmp(cmd, "cd") == 0){
    	return builtin_cd(argv);
    } else if( strcmp(cmd, "pwd") == 0){
        return builtin_pwd();
    } else if( strcmp(cmd, "prt") == 0){
        return builtin_prt();
    } else if( strcmp(cmd, "chpmt") == 0){
    	return builtin_chpmt(argv);
    } else if( strcmp(cmd, "chclr") == 0){
    	return builtin_chclr(argv);
    } else {
    	error("Not a bulitin cmd:%s\n", cmd);
    	return SF_FAIL;	
    }

    return SF_SUCCESS;    
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
	debug("exeCmd(%d, %s, ..)\n", argc, argv[0]);

	if(isPath(argv[0]) == SF_FALSE){
		// not a path
		debug("*isPath:False\n");

		if(existsInPath(argv[0]) != SF_FALSE){

			debug("EXECUTE pid=%d\n", getpid());

			int i = 0;
			while(argv[i] != 0){
				debug("argc=%d, argv[%d]=%s\n", argc, i, argv[i]);
				i++;
			}

			ret = execvp(argv[0], argv);

			if(ret == -1){
				perror("exeCmd failed");
			}

		} else {
			fprintf(stderr, "No such command!\n");
			return 1; //TODO
		}		
	} else {
		// is path
		debug("*isPath:True\n");
		
		ret = execv((const char*)argv[0], argv);

		if(ret == -1){
			//TODO
		}

		return 0; //TODO
	}

    return ret;
}

int isBuiltin(char* argv_0){
	char* cmd = argv_0;
	if(argv_0 == NULL){
		return -1;
	}

	if( strcmp(cmd, "help") == 0 ||
		strcmp(cmd, "exit") == 0 || 
		strcmp(cmd, "cd") == 0 ||
    	strcmp(cmd, "pwd") == 0 || 
        strcmp(cmd, "prt") == 0 ||
        strcmp(cmd, "chpmt") == 0 ||
    	strcmp(cmd, "chclr") == 0 ){
    	return !SF_FALSE;
    }

    return SF_FALSE;
}

int isBgProc(char* cmd){
    return SF_FALSE;
}

/* Pipeline */
int getNextPipe(int argc, char** argv, int from){
	int i;
	for(i = from; i < argc - 1; i++){ // -1 because don't want to check the last null argv
		if( strcmp(argv[i], LT_PIPE) == 0 ||
			strcmp(argv[i], JS_PIPE) == 0 ||
			strcmp(argv[i], GT_PIPE) == 0 ||
			strcmp(argv[i], BG_RUN) == 0){
			return i;
		}
	}

	return -1;
}

void SetFd(int pipe_fd[2]){
    if(pipe_fd[READ_END] != STDIN_FILENO){
        int ret = dup2(pipe_fd[READ_END], STDIN_FILENO);
        if(ret == -1){
            fprintf(stderr, "dup2(pipe_fd[READ_END]=%d, STDIN_FILENO=%d) failed\n",pipe_fd[READ_END], STDIN_FILENO);
            perror("dup2 for read_end");
            last_exe.val = ret;
            exit(last_exe.val); //TODO
        }
    }

    if(pipe_fd[WRITE_END] != STDOUT_FILENO){
        int ret = dup2(pipe_fd[WRITE_END], STDOUT_FILENO);
        if(ret == -1){
            fprintf(stderr, "dup2(pipe_fd[READ_END]=%d, STDIN_FILENO=%d) failed\n",pipe_fd[READ_END], STDIN_FILENO);
            perror("dup2 for write_end");
            last_exe.val = ret;
            exit(last_exe.val);
        }
    }
}

void CloseFd(int pipe_fd[2]){
	if(pipe_fd[READ_END] != STDIN_FILENO){
	    close(pipe_fd[READ_END]);
	}

	if(pipe_fd[WRITE_END] != STDOUT_FILENO){
	    close(pipe_fd[WRITE_END]);
	}
}

void HandleExit(pid_t wpid, int childStatus){
	if(WIFEXITED(childStatus)){
        last_exe.val = WEXITSTATUS(childStatus);
        debug("Child %d terminated with exit code %d\n", wpid, last_exe.val);
    } else{
        last_exe.val = WEXITSTATUS(childStatus);
        debug("Child %d terminated abnormally: %d\n", wpid, last_exe.val);
    }
}