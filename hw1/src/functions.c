/*functions.c*/
#include "map_reduce.h"

// TODO
/** It is used for validateargs
* @return Returns 0 if the directory is not available
*                 1 if the directory is available
*/
int isValidDir(char* dir){
    DIR *pDir;

    // open directory
    pDir = opendir(dir);

    if(NULL== pDir){ // opening failed
        return 0;
    }
    return 1;
}

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
    // fprintf(stderr, "argc: %d\n", argc);
    // for(int i = 0; i < argc; i++){
        // fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    // }
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
        if(strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0){
            // fprintf(stderr, "Skipped : %s\n", pDirent->d_name);
        }
        else{
            count++;
        }
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
int map(char* dir, void* results, size_t size, int (*act)( FILE * f, void * res, char * fn)){
    struct dirent *pDirent;
    DIR *pDir;
    int toRet, actRet, countLoop;

    // fprintf(stderr, "* map(%s, %p, %lu, act)\n", dir, results, size);
    // open directory
    pDir = opendir(dir);

    if(pDir == NULL){ // opening failed
        return -1;
    }

    // read file for each directory
    toRet = 0;
    countLoop = 0;
    while ((pDirent = readdir(pDir)) != NULL) {
        if(strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0){
            // fprintf(stderr, "* Skipped : %s\n", pDirent->d_name);
            continue;
        }
        
        // 2. Create relative path from argument and pDirent->d_name
        int filenameLength = strlen(dir) + strlen(pDirent->d_name) + 3;
        char* fullPath = (char *) malloc(filenameLength);
        strncat(fullPath, dir, strlen(dir));
        strncat(fullPath, "/", strlen("/"));
        strncat(fullPath, pDirent->d_name, strlen(pDirent->d_name));

        /*[DEBUG] START*/
        // fprintf(stderr, "\n* Dir of file is %s, Name of file is %s\n",  dir, pDirent->d_name);
        // fprintf(stderr, "* Total of %lu length including the / & null terminator\n", strlen(fullPath));
        // fprintf(stderr, "* New filepath is %s\n", fullPath);
        /*[DEBUG] END*/

        // 3. Open File
        FILE* openedFile = fopen(fullPath, "r");
        if(openedFile == NULL){
            free(fullPath);
            return -1;
        }

        // 4. Do actions
        void* resultAddr = (void*)((long)results + countLoop * size);
        // fprintf(stderr, "* Right before calling act(FILE, %p, %s)\n", resultAddr, pDirent->d_name);

        actRet = act(openedFile, resultAddr, pDirent->d_name);
        // fprintf(stderr, "* actRet: %d\n", actRet);

        toRet = toRet + actRet;
        countLoop++;
        // fprintf(stderr, "* toRet: %d\n", toRet);
        free(fullPath);
    }

    // close directory
    closedir (pDir);

    // return count;
    return toRet;
}

/**
 * This reduce function takes the results produced by map and cumulates all
 * the data to give one final Analysis struct. Final struct should contain
 * filename of file which has longest line.
 *
 * @param  n         The number of files analyzed.
 * @param  results   The results array that has been populated by map.
 * @return The struct containing all the cumulated data.
*/
struct Analysis analysis_reduce(int n, void* results){
    int i, j;
    struct Analysis ana_results, longest_result;

    if(n <= 0) // in case n is 0
        return longest_result;

    // initialize longest_result to be the first result
    longest_result = *((struct Analysis*) results);
    // index = 0; // init index of the longest to 0

    for(i = 1; i < n; i++){
        ana_results = *( ((struct Analysis*) results) + i );
        if(ana_results.lnlen > longest_result.lnlen){
            longest_result = ana_results;
        }
        for(j = 0; j < 128; j++){
            longest_result.ascii[j] += ana_results.ascii[j];
        }
    }

    return longest_result;
}

/**
 * This reduce function takes the results produced by map and cumulates all
 * the data to give one final Stats struct. Filename field in the final struct 
 * should be set to NULL.
 *
 * @param  n       The number of files analyzed.
 * @param  results The results array that has been populated by map.
 * @return         The struct containing all the cumulated data.
 */
Stats stats_reduce(int n, void* results){
    int i, j;
    Stats stats_results, current_result;

    for(int i = 0; i < NVAL; i++){
        stats_results.histogram[i] = 0;
    }
    stats_results.sum = 0;
    stats_results.n = 0;
    stats_results.filename = NULL;

    if(n <= 0) // in case n is 0
        return stats_results;

    for(i = 0; i < n; i++){
        current_result = *( ((Stats*) results) + i );
        
        for(j = 0; j < NVAL; j++){
            stats_results.histogram[j] += current_result.histogram[j];
        }
         
        stats_results.sum += current_result.sum;
        stats_results.n += current_result.n;
    }

    return stats_results;
}

/**
 * Always prints the following:
 * - The name of the file (for the final result the file with the longest line)
 * - The longest line in the directory's length.
 * - The longest line in the directory's line number.
 *
 * Prints only for the final result:
 * - The total number of bytes in the directory.
 *
 * If the hist parameter is non-zero print the histogram of ASCII character
 * occurrences. When printing out details for each file (i.e the -v option was
 * selected) you MUST NOT print the histogram. However, it MUST be printed for
 * the final result.
 *
 * Look at sample output for examples of how this should be print. You have to
 * match the sample output for full credit.
 *
 * @param res    The final result returned by analysis_reduce
 * @param nbytes The number of bytes in the directory.
 * @param hist   If this is non-zero, prints additional information. (Only non-
 *               zero for printing the final result.)
 */
void analysis_print(struct Analysis res, int nbytes, int hist){
    int i, j;
    fprintf(stdout, "File: %s\n", res.filename);
    fprintf(stdout, "Longest line length: %d\n", res.lnlen);
    fprintf(stdout, "Longest line number: %d\n", res.lnno);
    fprintf(stdout, "Total Bytes in directory: %d\n", nbytes);
    if(hist != 0){
        for(i = 0; i < 128; i++){
            if(res.ascii[i] != 0){
                fprintf(stdout, "%d:", i);
                if(res.ascii[i] > 100)
                    fprintf(stdout, "Error: %d\n", res.ascii[i]);
                else
                    for(j = 0; j < res.ascii[i]; j++)
                        fprintf(stdout, "-");
                fprintf(stdout, "\n");
            }
        }
    }
    fprintf(stdout, "\n");
}

/**
 * Always prints the following:
 * Count (total number of numbers read), Mean, Mode, Median, Q1, Q3, Min, Max
 *
 * Prints only for each Map result:
 * The file name
 *
 * If the hist parameter is non-zero print the the histogram. When printing out
 * details for each file (i.e the -v option was selected) you MUST NOT print the
 * histogram. However, it MUST be printed for the final result.
 *
 * Look at sample output for examples of how this should be print. You have to
 * match the sample output for full credit.
 *
 * @param res  The final result returned by stats_reduce
 * @param hist If this is non-zero, prints additional information. (Only non-
 *             zero for printing the final result.)
 */
void stats_print(Stats res, int hist){
    int i, j;
    double mostFreqCnt, min, max, cur_pos, q1, q2, q3;
    double q1Cnt, q2Cnt, q3Cnt;

    // print histogram first
    if(hist != 0){
        for(i = 0; i < NVAL; i++){
            if(res.histogram[i] != 0){
                printf("%d:", i);
                for(j = 0; j < res.histogram[i]; j++)
                    printf("-");
                printf("\n");
            }
        }
        printf("\n");
    }

    if(res.filename != NULL){
        printf("File: %s\n", res.filename);
    }

    q1 = -1;
    q2 = -1;
    q3 = -1;

    q1Cnt = res.n * 0.25;
    q2Cnt = res.n * 0.5;
    q3Cnt = res.n * 0.75;

    cur_pos = 0;
    mostFreqCnt = 0;

    min = -1;
    max = -1;
    for(i = 0; i < NVAL; i++){
        if(res.histogram[i] != 0){
            if(min == -1){ // if this value is the first time that sth exists
                min = i; // update to the new min
            }

            if(max < i){ // if this value is larger than max
                max = i; // update to the new max
            }

            if(res.histogram[i] > mostFreqCnt){
                mostFreqCnt = res.histogram[i];
            }

            cur_pos += res.histogram[i];
            if(cur_pos >= q1Cnt && q1 == -1){
                q1 = i;
            }
            if(cur_pos >= q2Cnt && q2 == -1){
                q2 = i;
            }
            if(cur_pos >= q3Cnt && q3 == -1){
                q3 = i;
            }

        }
    }
    printf("Count: %d\n", res.n);
    printf("Mean: %f\n", ((double)res.sum/(double)res.n));
    printf("Mode: ");
    for(i = 0; i < NVAL; i++){
        if(res.histogram[i] != 0 && res.histogram[i] == mostFreqCnt){
            printf("%d ", i);
        }
    }
    printf("\n");
    printf("Median: %f\n", q2);

    printf("Q1: %f\n", q1);
    printf("Q3: %f\n", q3);
    printf("Min: %f\n", min);
    printf("Max: %f\n", max);
    printf("\n");
}

/**
 * This function performs various different analyses on a file. It
 * calculates the total number of bytes in the file, stores the longest line
 * length and the line number, and frequencies of ASCII characters in the file.
 *
 * @param  f        The filestream on which the action will be performed. You
 *                  you can assume the filestream passed by map will be valid.
 * @param  res      The slot in the results array in which the data will be
 *                  stored.
 * @param  filename The filename of the file currently being processed.
 * @return          Return the number of bytes read.
 */
int analysis(FILE* f, void* res, char* filename){
    int bufferSize, totalRead, freadRet, lineNo, cntPerLine;
    char* buffer;

    // fprintf(stderr, "* analysis(FILE, %p, %s)\n", res, filename);    

    // CONSTANT
    bufferSize = 100;

    // VARIABLE
    totalRead = 0; // keep track of number of bytes read
    lineNo = 1; // line number (i.e., increased when '\n' is found)
    cntPerLine = 0; // char counter for one line (i.e., init to 0 when '\n' is found)    

    // initialize the result structure
    ((struct Analysis*) res)->lnlen = 0;
    ((struct Analysis*) res)->lnno = 0;
    ((struct Analysis*) res)->filename = "";
    for(int i = 0; i < 128; i++){
        ((struct Analysis*) res)->ascii[i] = 0;
    }

    // allocate buffer
    buffer = (char*) malloc(bufferSize * sizeof(char));

    // set filename
    ((struct Analysis*) res)->filename = strdup(filename);

    do{
        freadRet = fread(buffer, sizeof(char), bufferSize, f);

        // [DEBUG] Start
        // fprintf(stderr, "Current freadRet: %d\n", freadRet);
        // fprintf(stderr, "Read line %d: ", lineNo);
        // [DEBUG] END

        for(int i = 0; i < freadRet; i++){
            ((struct Analysis*) res)->ascii[(int)buffer[i]]++;
            // [DEBUG] Start
            // fprintf(stderr, "%c(%d)", buffer[i], buffer[i]);
            // [DEBUG] END

            if(buffer[i] == '\n'){
                if(cntPerLine > ((struct Analysis*) res)->lnlen){

                    // update 
                    ((struct Analysis*) res)->lnlen = cntPerLine;
                    ((struct Analysis*) res)->lnno = lineNo;
                }

                // initialize counting var
                cntPerLine = 0;

                // update line number
                lineNo++;

                // [DEBUG] Start
                // fprintf(stderr, "\n");
                // [DEBUG] END
            }
            else {
                // increase the count
                cntPerLine++;
            }
        }

        // update totalRead
        totalRead += freadRet;
    }while(freadRet == bufferSize);
    
    // free pointers
    free(buffer);
    fclose(f);

    return totalRead;
}

/**
 * This function counts the number of occurrences of each number in a file. It
 * also calculates the sum total of all numbers in the file and how many numbers
 * are in the file. If the file has an invalid entry return -1.
 *
 * @param  f        The filestream on which the action will be performed. You
 *                  you can assume the filestream passed by map will be valid.
 * @param  res      The slot in the results array in which the data will be
 *                  stored.
 * @param  filename The filename of the file currently being processed.
 * @return          Return 0 on success and -1 on failure.
 */
int stats(FILE* f, void* res, char* filename){
    int fscanfRet, readVal;

    // initialize the result structure
    for(int i = 0; i < NVAL; i++){
        ((Stats*) res)->histogram[i] = 0;
    }
    ((Stats*) res)->sum = 0;
    ((Stats*) res)->n = 0;
    ((Stats*) res)->filename = "";

    // set filename
    ((Stats*) res)->filename = strdup(filename);

    do{
        fscanfRet = fscanf(f, "%d", &readVal);

        if(fscanfRet == EOF){
            break;
        }

        if(readVal < 0 || readVal > NVAL){
            continue;
        }

        // [DEBUG] Start
        // fprintf(stderr, "Current fscanfRet: %d\n", readVal);
        // [DEBUG] END

        ((Stats*) res)->n++;
        ((Stats*) res)->sum += readVal;
        ((Stats*) res)->histogram[readVal]++;

        // [DEBUG] Start
        // fprintf(stderr, "%d ", readVal);
        // [DEBUG] END

    }while(fscanfRet != EOF);

    // for(int i = 0; i < NVAL; i++){
    //     fprintf(stderr, "[%d]: %d\n", i, ((Stats*) res)->histogram[i]);
    // }
    // fprintf(stderr, "sum: %d\n", ((Stats*) res)->sum);
    // fprintf(stderr, "n  : %d\n", ((Stats*) res)->n);
    // fprintf(stderr, "char: %s\n", ((Stats*) res)->filename);
    
    // free pointers
    fclose(f);

    return 0;
}