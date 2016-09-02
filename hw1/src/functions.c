/*functions.c*/
#include "main.h"
/**
* Validates the command line arguments passed in by the user.
* @param  argc The number of arguments.
* @param  argv The arguments.
* @return Returns -1 if arguments are invalid (refer to hw document).
* Returns 0 if -h optional flag is selected. Returns 1 if analysis
* is chosen. Returns 2 if stats is chosen. If the -v optional flag
* has been selected, validateargs returns 3 if analysis
* is chosen and 4 if stats is chosen.
*/
int validateargs(int argc, char** argv){
    /*[DEBUG] Start*/
    fprintf (stderr, "argc: %d\n", argc);
    for(int i = 0; i < argc; i++){
        fprintf (stderr, "argv[%d]: %s\n", i, argv[i]);
    }
    /*[DEBUG] Done*/

    if(argc > 4 || argc <= 1)
        return -1; // too many/few args

    int ret = 0; // the minimum return value

    // if code reached here, it means that there is at least two args but 4 at the most

    int indexOfFunc;
    if(strcmp("-h", argv[1]) == 0) { // if argv[1] is "-h"
        return ret;
    } else if(strcmp("-v", argv[1]) == 0){
        indexOfFunc = 2;
        ret += 2;
    } else {
        indexOfFunc = 1;
    }
    // fprintf(stderr, "indexOfFunc is now %d\n", indexOfFunc);

    if(strcmp("ana", argv[indexOfFunc]) == 0 && isValidDir(argv[indexOfFunc + 1])==1){ // if the function is "ana" and directory is valid
        ret += 1;
    } else if(strcmp("stats", argv[indexOfFunc]) == 0 && isValidDir(argv[indexOfFunc + 1])==1){
        ret += 2;
    } else {
        return -1;  // invalid dir
    }
    
    return ret;
}

// TODO
/** It is used for validateargs
* @return Returns 0 if the directory is not available
*                 1 if the directory is available
*/
int isValidDir(char* dir){
    return 1;
}


/**
* Counts the number of files in a directory EXCLUDING . and ..
* @param  dir The directory for which number of files is desired.
* @return The number of files in the directory EXCLUDING . and ..
* If nfiles returns 0, then print "No files present in the
* directory." and the program should return EXIT_SUCCESS.
* Returns -1 if any sort of failure or error occurs.
*/
int nfiles(char* dir){
    int count;
    struct dirent *pDirent;
    DIR *pDir;

    // initialize variables
    count = 0;

    // open directory
    pDir = opendir(dir);

    if(pDir == NULL){ // opening failed
        return -1;
    }

    // read file for each directory
    while ((pDirent = readdir(pDir)) != NULL) {
        if(strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0)
            fprintf (stderr, "Skipped : %s\n", pDirent->d_name);
        else
            count++;
    }

    // close directory
    closedir (pDir);

    return count;
}

/**
* The map function goes through each file in a directory, performs some action on
* the file and then stores the result.
*
* @param  dir     The directory that was specified by the user.
* @param  results The space where map can store the result for each file.
* @param  size    The size of struct containing result data for each file.
* @param  act     The action (function map will call) that map will perform on
* each file. Its argument f is the file stream for the specific
* file. act assumes the filestream is valid, hence, map should
* make sure of it. Its argument res is the space for it to store
* the result for that particular file. Its argument fn is a
* string describing the filename. On failure returns -1, on
* sucess returns value specified in description for the act
* function.
*
* @return The map function returns -1 on failure, sum of act results on
* success.
*/
int map(char* dir,void* results, size_t size, int (*act)( FILE * f, void * res, char * fn));