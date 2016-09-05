#!/bin/sh
#test_analysis_reduce.sh
#NOT WORKING
BINARY_FILE="test.out"
TEMP_FOLDER="temp"

(cat << END > tester.c
#include "../include/map_reduce.h"
//Space to store the results for analysis map
struct Analysis analysis_space[3];
//Space to store the results for stats map
//Stats stats_space[NFILES];

int main(int argc, char** argv) {
	int nfile = nfiles(argv[1]);
    int validationValue = map(argv[1], analysis_space, nfile, analysis);
    fprintf(stderr, "validationValue is %d\n", validationValue);
    return 0;
}
END
)

gcc -c functions.c
gcc -c tester.c
gcc functions.o tester.o -o $BINARY_FILE

# test case 1
mkdir $TEMP_FOLDER
echo "Hello this is file1.txt" > $TEMP_FOLDER/file1.txt
echo "Hello this is file22.txt" > $TEMP_FOLDER/file2.txt
echo "Hello this is file333.txt" > $TEMP_FOLDER/file3.txt
./$BINARY_FILE $TEMP_FOLDER
echo ""

# test case 2
# touch $TEMP_FOLDER/file4.txt
# touch $TEMP_FOLDER/file5.txt
# touch $TEMP_FOLDER/file6.txt
# ./$BINARY_FILE $TEMP_FOLDER
# echo ""

# # test case 3
# rm $TEMP_FOLDER/*
# echo "Expected 0"
# ./$BINARY_FILE $TEMP_FOLDER
# echo ""

# # test case 4
# rm -rf $TEMP_FOLDER
# echo "Expected -1"
# ./$BINARY_FILE $TEMP_FOLDER
# echo ""

# clean up
rm *.o
rm $BINARY_FILE
rm -rf $TEMP_FOLDER
# rm tester.c