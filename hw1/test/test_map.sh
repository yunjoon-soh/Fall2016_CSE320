#!/bin/sh
#test_map.sh
BINARY_FILE="./test.out"
TEMP_FOLDER="temp"

(cat << END > tester.c
#include "main.h"
//Space to store the results for analysis map
//struct Analysis analysis_space[NFILES];
//Space to store the results for stats map
//Stats stats_space[NFILES];
int cat(FILE* f, void* res, char* filename) {
	char c;
	int n = 0;
	printf("================================START of cat()\n");
	printf("%s\n", filename);
		while((c = fgetc(f)) != EOF) {
	    printf("%c", c);
	    n++;
	}
	printf("================================END of cat()\n");
	return n;
}
int main(int argc, char** argv) {
    int validationValue = map(argv[1], 0, 0, cat);
    fprintf(stderr, "validationValue is %d\n", validationValue);
    return 0;
}
END
)

gcc -c map_reduce.c -I../include/
gcc -c tester.c -I../include
gcc map_reduce.o tester.o -o $BINARY_FILE

# test case 1
mkdir $TEMP_FOLDER
echo "Hello this is file1.txt" > $TEMP_FOLDER/file1.txt
echo "Hello this is file2.txt" > $TEMP_FOLDER/file2.txt
echo "Hello this is file3.txt" > $TEMP_FOLDER/file3.txt
$BINARY_FILE $TEMP_FOLDER
echo ""

# test case 2
touch $TEMP_FOLDER/file4.txt
touch $TEMP_FOLDER/file5.txt
touch $TEMP_FOLDER/file6.txt
$BINARY_FILE $TEMP_FOLDER
echo ""

# # test case 3
# rm $TEMP_FOLDER/*
# echo "Expected 0"
# $BINARY_FILE $TEMP_FOLDER
# echo ""

# # test case 4
# rm -rf $TEMP_FOLDER
# echo "Expected -1"
# $BINARY_FILE $TEMP_FOLDER
# echo ""

# clean up
rm *.o
rm $BINARY_FILE
rm -rf $TEMP_FOLDER
# rm tester.c