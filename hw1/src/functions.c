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
    /*[DEBUG]*/
    fprintf (stderr, "argc: %d\n", argc);
    for(int i = 0; i < argc; i++){
        fprintf (stderr, "argv[%d]: %s\n", i, argv[i]);
    }

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

/**
* @return Returns 0 if the directory is not available
*                 1 if the directory is available
*/
int isValidDir(char* dir){
    return 1;
}
