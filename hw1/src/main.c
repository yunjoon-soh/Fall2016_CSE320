#include "map_reduce.h"

//Space to store the results for analysis map
struct Analysis analysis_space[NFILES];
//Space to store the results for stats map
Stats stats_space[NFILES];

//Sample Map function action: Print file contents to stdout and returns the number bytes in the file.
int cat(FILE* f, void* res, char* filename) {
    char c;
    int n = 0;
    printf("%s\n", filename);
    while((c = fgetc(f)) != EOF) {
        printf("%c", c);
        n++;
    }
    printf("\n");
    return n;
}

void printUsage(){
    fprintf(stderr, "Usage: ./mapreduce [h|v] FUNC DIR\n");
    fprintf(stderr, "\tFUNC\tWhich operation you would like to run on the data:\n");
    fprintf(stderr, "\t\tana - Analysis of various text files in a directory.\n");
    fprintf(stderr, "\t\tstats - Calculates stats on files which contain only numbers.\n");
    fprintf(stderr, "\tDIR\tThe directory in which the files are located.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "\tOptions:\n");
    fprintf(stderr, "\t\t-h\tPrints this help menu.\n");
    fprintf(stderr, "\t\t-v\tPrints the map function’s results, stating the file it’s from.\n");
}

int main(int argc, char** argv) {
    struct Analysis map_reduce;
    Stats stats_reduce_result;
    int numOfFiles, i, mapRet;
    char* dir;
    // fprintf(stderr, "Initiating... %s\n", argv[0]);

    // Validate
    int validationValue = validateargs(argc, argv);
    // fprintf(stderr, "validationValue is %d\n", validationValue);

    // Set directory name
    switch(validationValue){
    case -1:
        printUsage();
        return -1; //TODO return on failure
    case 0:
        printUsage();
        return 0;
    case 1:
    case 2:
        dir = argv[2]; // without -v
        break;
    case 3:
    case 4:
        dir = argv[3];
        break;
    }

    // Count files
    numOfFiles = nfiles(dir);
    // fprintf(stderr, "numOfFiles: %d\n", numOfFiles);

    // call map()
    switch(validationValue){
    // case 0: // should not be reached here
    case 1:
        mapRet = map(dir, &analysis_space, sizeof(struct Analysis), analysis);
        
        map_reduce = analysis_reduce(numOfFiles, &analysis_space);

        analysis_print(map_reduce, mapRet, 1);

        break;
    case 2:
        mapRet = map(dir, &stats_space, sizeof(Stats), stats);

        stats_reduce_result = stats_reduce(numOfFiles, &stats_space);

        stats_print(stats_reduce_result, 1);

        break;
    case 3:
        mapRet = map(dir, &analysis_space, sizeof(struct Analysis), analysis);
        
        for(i = 0; i < numOfFiles; i++){
            analysis_print(analysis_space[i], mapRet, 0);           
        }

        map_reduce = analysis_reduce(numOfFiles, &analysis_space);

        analysis_print(map_reduce, mapRet, 1);
        break;
    case 4:
        mapRet = map(dir, &stats_space, sizeof(Stats), stats);

        for(i = 0; i < numOfFiles; i++){
            stats_print(stats_space[i], 0);         
        }

        stats_reduce_result = stats_reduce(numOfFiles, &stats_space);

        stats_print(stats_reduce_result, 1);
        break;
    }

    return 0;
}
