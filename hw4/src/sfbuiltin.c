#include "sfbuiltin.h"
#include "sfconst.h"

#ifndef DEBUG
#define DEBUG
#endif

#define USAGE_LENGTH 7
const char* USAGE[USAGE_LENGTH] = {
	"help\n",
	"exit\n",
	"cd\n",
	"pwd\n",
	"prt\n",
	"chpmt\n",
	"chclr\n"
};

int preprocess(){
	debug("Preprocess called\n");

    // get current dir and set it as cd_history
    getcwd(cd_history, PATH_MAX);
    if(cd_history == NULL){
        perror("Failed: getcwd...");
        return SF_FAIL;
    }

    // initialize last_execution result
    last_exe.val = 0;

    HOME_DIR = getenv("HOME");

    PROMPT_USER = PROMPT_ENABLED;
    PROMPT_HOST = PROMPT_ENABLED;

    PROMPT_BOLD_USER = NOML;
    PROMPT_BOLD_HOST = NOML;
    PROMPT_COLOR_USER = KNRM;
    PROMPT_COLOR_HOST = KNRM;

    return SF_SUCCESS;
}

int countArgs(char** argv, int index){
	int cnt = 0;
	while(argv[index] != 0){
		index++; cnt++;
	}
	return cnt;
}

int builtin_help(){
	debug("print help\n");
	int i;
	for(i = 0; i < USAGE_LENGTH; i++){
		printf("%s", USAGE[i]); 
	}

	return SF_SUCCESS;
}

int builtin_exit(){
	debug("exiting\n");
	return SF_SUCCESS;
}

// if path == NULL, "cd" was typed
// if path is not null, null terminated string
int builtin_cd(char** argv){
	char *path;
	int ret;
	char *last_backslash;
	char cdir[PATH_MAX];

	int argc = countArgs(argv, 0);
	if(argc < 2){
		debug("Path is given NULL... changing directory to home dir\n");
		path = getenv("HOME");
	} else {
		path = argv[1];
	}

	debug("argc=%d, Path:%s, cd_history:%s\n", argc, path, cd_history);

	// get current dir
	getcwd(cdir, PATH_MAX);
	if(cdir == NULL){
		fprintf(stderr, "Failed: getcwd...\n");
		return SF_FAIL;
	}

	// set path for special cases
	if( strcmp(path, "..") == 0 ){
		debug("pdir\n");
		path = &cdir[0];
		last_backslash = strrchr(path, '/'); 
		if (last_backslash){
			*last_backslash = '\0';
		}

	} else if( strcmp(path, ".") == 0 ){
		debug("cdir\n");
		
		return SF_SUCCESS;// path is current dir

	} else if( strcmp(path, "-") == 0 ){
		debug("prev_dir\n");
		path = cd_history;
		// prev_dir = !SF_FALSE;
	}

	debug("Check if target folder exists\n");
	struct stat stat_res; // stat result
	ret = stat(path, &stat_res);
	if(ret == -1){
		perror("stat");
		fprintf(stderr, "%s\n", path);
		return SF_FAIL;
	} else if(ret == 0){
		if(!S_ISDIR(stat_res.st_mode)){
			fprintf(stderr, "%s: Not a directory", path);
			return SF_FAIL;
		}
	}

	debug("Try change directory to... %s\n", path);
	ret = chdir(path);

	if(ret == 0){
		// // save history
		// if( strcmp(path, "-") == 0 ){
		// 	// if "-" save current dir as history
		// 	strncpy(cd_history, cdir, strlen(cdir) + 1);
		// } else{
		// 	// if not "-", save the 
		// 	strncpy(cd_history, , strlen(path) + 1);
		// }
		strncpy(cd_history, cdir, strlen(cdir) + 1);

		debug("Changing dir successful(cd_history=%s)\n", cd_history);

		return SF_SUCCESS;
	} else if(ret == -1){
		debug("Changing dir unsuccessful\n");
		perror("chdir");
		return SF_FAIL;
	} else{
		error("Return value of chdir(%s)=%d, not 0 nor -1...\n", path, ret);
		return SF_FAIL;
	}
}

int builtin_pwd(){
	char cwd[PATH_MAX], * ret;
	ret = getcwd(cwd, PATH_MAX);
	if(ret == NULL){
		error("getcwd failed");
		return SF_FAIL;
	} else{
		printf("%s\n", ret);
		return SF_SUCCESS;
	}
}

int builtin_prt(){
	printf("Last executed result: %d\n", last_exe.val);
	return SF_SUCCESS;
}

int builtin_chpmt(char** argv){
	debug("chpmt called\n");
	int argc = countArgs(argv, 0);
	if(argc != 3){
		fprintf(stderr, "Error: chpmt: Not enough arguments\n");
		return SF_FAIL;
	}

	if( strcmp(argv[2], "0") != 0 && strcmp(argv[2], "1") != 0){
		fprintf(stderr, "Error: chpmt: second argument has to be 0 or 1\n");
		return SF_FAIL;
	}

	if( strcmp(argv[1], "user") == 0){
		PROMPT_USER = *argv[2] - '0';
	} else if (strcmp(argv[1], "machine") == 0){
		PROMPT_HOST = *argv[2] - '0';
	} else {
		fprintf(stderr, "Error: chpmt: invalid first arugment\n");
		return SF_FAIL;
	}

	return SF_SUCCESS;
}

int builtin_chclr(char** argv){
	int argc = countArgs(argv, 0);
	if(argc != 4){
		fprintf(stderr, "Error: chclr: Not enough arguments\n");
		return SF_FAIL;
	}

	char *bold;	
	if( strcmp(argv[2], "0") == 0 ){
		bold = NOML;
	} else if( strcmp(argv[2], "1") == 0 ){
		bold = BOLD;
	} else {
		fprintf(stderr, "Error: chclr: third argument has to be 0 or 1\n");
		return SF_FAIL;
	}

	char *color;
	if( strcmp(argv[3], "red") == 0){
		color = KRED;
	} else if( strcmp(argv[3], "blue") == 0){
		color = KBLU;
	} else if( strcmp(argv[3], "green") == 0){
		color = KGRN;
	} else if( strcmp(argv[3], "yellow") == 0){
		color = KYEL;
	} else if( strcmp(argv[3], "cyan") == 0){
		color = KCYN;
	} else if( strcmp(argv[3], "magenta") == 0){
		color = KMAG;
	} else if( strcmp(argv[3], "black") == 0){
		color = KBLK;
	} else if( strcmp(argv[3], "white") == 0){
		color = KWHT;
	} else {
		fprintf(stderr, "%s\n", "Error: chclr: second argument is invalid");
		return SF_FAIL;
	}

	if( strcmp(argv[1], "user") == 0){
		PROMPT_COLOR_USER = color;
		PROMPT_BOLD_USER = bold;
	} else if (strcmp(argv[1], "machine") == 0){
		PROMPT_COLOR_HOST = color;
		PROMPT_BOLD_HOST = bold;
	} else {
		fprintf(stderr, "Error: chclr: invalid first arugment\n");
		return SF_FAIL;
	}

	return SF_SUCCESS;
}