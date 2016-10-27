#include "sfbuiltin.h"
#include "sfconst.h"

#define USAGE_LENGTH 12
const char* USAGE[USAGE_LENGTH] = {
	"Command line utility for converting files from UTF-16LE to UTF-16BE or vice versa.\n\n",
	"Usage:\n",
	"\t./utf [-h|--help] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]\n",
	"\t\t-h, --help\tDisplays this usage.\n",
	"\t\t-v, -vv   \tToggles the verbosity of the program to level 1 or 2.\n\n",
	"\tMandatory argument:\n",
	"\t\t-u OUT_ENC, --UTF=OUT_ENC\t Sets the output encoding.\n",
	"\t\t\t 8\tUTF-8\n",
	"\t\t\t 16LE\tUTF-16 Little Endian\n",
	"\t\t\t 16BE\tUTF-16 Big Endian\n\n"
	"\tPositional Arguments:\n",
	"\t\tIN_FILE   \tThe file to convert.\n",
	"\t\t[OUT_FILE]\tOutput file name. If not present, defaults to stdout.\n"
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

    return SF_SUCCESS;
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
int builtin_cd(char* path){
	int ret;
	char *last_backslash;
	char cdir[PATH_MAX];

	// prev_dir = SF_FALSE; // flag for "cd -"
	debug("Path:%s, cd_history:%s\n", path, cd_history);

	if(path == NULL){
		debug("Path is given NULL... changing directory to home dir\n");
		path = getenv("HOME");
	}

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
		error("pwd failed");
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