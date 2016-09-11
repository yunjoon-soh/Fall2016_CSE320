#!/bin/sh
#test_analysis_reduce.sh
#NOT WORKING
BINARY_FILE="./test.out"
TEMP_FOLDER="temp"

(cat << END > tester.c
#include "../include/map_reduce.h"
//Space to store the results for analysis map
struct Analysis analysis_space[3];
//Space to store the results for stats map
//Stats stats_space[NFILES];

int main(int argc, char** argv) {
	int mapRet;
	struct Analysis map_reduce;
	int nfile = nfiles(argv[1]);

    mapRet = map(argv[1], &analysis_space, sizeof(struct Analysis), analysis);
    fprintf(stderr, "mapRet: %d\n", mapRet);
        
	map_reduce = analysis_reduce(nfile, &analysis_space);

    analysis_print(map_reduce, mapRet, 1);

    return 0;
}
END
)

gcc -c ../src/map_reduce.c -I../include/
gcc -c ./tester.c -I../include
gcc map_reduce.o tester.o -o $BINARY_FILE

# test case 1
echo "Test Case #1"
mkdir -p $TEMP_FOLDER
echo "Hello this is file1.txt" > $TEMP_FOLDER/file1.txt
echo "Hello this is file22.txt" > $TEMP_FOLDER/file2.txt
echo "Hello this is file333.txt" > $TEMP_FOLDER/file3.txt
./$BINARY_FILE $TEMP_FOLDER
echo ""

#test case 2
echo "Test Case #2"
touch $TEMP_FOLDER/file4.txt
touch $TEMP_FOLDER/file5.txt
touch $TEMP_FOLDER/file6.txt
./$BINARY_FILE $TEMP_FOLDER
echo ""

# test case 3
echo "Test Case #3"
rm $TEMP_FOLDER/*
echo "Expected 0"
./$BINARY_FILE $TEMP_FOLDER
echo ""

# test case 4
echo "Test Case #4"
rm -rf $TEMP_FOLDER
echo "Expected -1"
./$BINARY_FILE $TEMP_FOLDER
echo ""

# test case 5
echo "Test Case #5"
rm -rf $TEMP_FOLDER
mkdir -p $TEMP_FOLDER
touch $TEMP_FOLDER/file1.txt
./$BINARY_FILE $TEMP_FOLDER
echo ""

# clean up
rm *.o
rm $BINARY_FILE
rm -rf $TEMP_FOLDER
rm tester.c