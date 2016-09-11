#!/bin/sh
BINARY_FILE="./test.out"
TEMP_FOLDER="temp"

(cat << EOF 
#include "map_reduce.h"
int main(int argc, char** argv) {
    int validationValue = nfiles(argv[1]);
    fprintf(stderr, "number of files: %d", validationValue);
    return 0;
}
EOF
) > tester.c

gcc -c ../src/map_reduce.c -I../include/
gcc -c ./tester.c -I../include
gcc map_reduce.o tester.o -o $BINARY_FILE

# test case 1
mkdir $TEMP_FOLDER
touch $TEMP_FOLDER/file1.txt
touch $TEMP_FOLDER/file2.txt
touch $TEMP_FOLDER/file3.txt
echo "Expected 3"
$BINARY_FILE $TEMP_FOLDER
echo ""

# test case 2
touch $TEMP_FOLDER/file4.txt
touch $TEMP_FOLDER/file5.txt
touch $TEMP_FOLDER/file6.txt
echo "Expected 6"
$BINARY_FILE $TEMP_FOLDER
echo ""

# test case 3
rm $TEMP_FOLDER/*
echo "Expected 0"
$BINARY_FILE $TEMP_FOLDER
echo ""

# test case 4
rm -rf $TEMP_FOLDER
echo "Expected -1"
$BINARY_FILE $TEMP_FOLDER
echo ""

# clean up
rm *.o
rm $BINARY_FILE
rm -rf $TEMP_FOLDER
rm tester.c