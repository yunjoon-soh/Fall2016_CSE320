#!/bin/sh
#test_total.sh
BINARY_FILE="test.out"
TEMP_FOLDER="temp"

(cat << END > tester.c
#include "../include/map_reduce.h"
//Space to store the results for analysis map
struct Analysis analysis_space[NFILES];

//Space to store the results for stats map
Stats stats_space[NFILES];
void printUsage(){
	//TODO
	fprintf(stderr, "This is usage\n");
}
int main(int argc, char** argv) {
	struct Analysis map_reduce;
	int numOfFiles, i, mapRet;
	char* dir;
    fprintf(stderr, "Initiating... %s\n", argv[0]);

    // Validate
    int validationValue = validateargs(argc, argv);
    fprintf(stderr, "validationValue is %d\n", validationValue);

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
    fprintf(stderr, "numOfFiles: %d\n", numOfFiles);

    // call map()
    switch(validationValue){
    // case 0: // should not be reached here
	case 1:
		mapRet = map(dir, &analysis_space, sizeof(struct Analysis), analysis);
		
		map_reduce = analysis_reduce(numOfFiles, &analysis_space);

		analysis_print(map_reduce, mapRet, 1);

		break;
	case 2:
	case 3:
		mapRet = map(dir, &analysis_space, sizeof(struct Analysis), analysis);
		
		for(i = 0; i < numOfFiles; i++){
			analysis_print(analysis_space[i], mapRet, 0);			
		}

		map_reduce = analysis_reduce(numOfFiles, &analysis_space);

		analysis_print(map_reduce, mapRet, 1);
	case 4:
		dir = argv[3];
		break;
    }

    return 0;
}
END
)

gcc -c functions.c -I../include/
gcc -c tester.c
gcc functions.o tester.o -o $BINARY_FILE

# test case #1
# echo "* Expected: Print usage and exit"
# ./$BINARY_FILE -h ana $TEMP_FOLDER

# test case #2
# echo "* Expected: Print usage and exit(no such folder)"
# ./$BINARY_FILE ana $TEMP_FOLDER

# test case #3
mkdir $TEMP_FOLDER
cp ../rsrc/ana_light/* ./$TEMP_FOLDER/
echo "* Expected: Run properly"
./$BINARY_FILE ana $TEMP_FOLDER
rm -rf $TEMP_FOLDER # clean

# ./$BINARY_FILE -v ana $TEMP_FOLDER

# clean up
rm *.o
rm $BINARY_FILE
rm -rf $TEMP_FOLDER
rm tester.c