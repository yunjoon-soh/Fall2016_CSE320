#!/bin/sh
#test_map.sh
BINARY_FILE="./test.out"
TEMP_FOLDER="temp"

(cat << END > tester.c
#include "map_reduce.h"
//Space to store the results for analysis map
//struct Analysis analysis_space[NFILES];
//Space to store the results for stats map
//Stats stats_space[NFILES];

int main(int argc, char** argv) {
	Stats test;
	
	FILE* f = fopen(argv[1], "r");

    int validationValue = stats(f, &test, argv[1]);
    fprintf(stderr, "stats value is %d\n", validationValue);

    stats_print(test, 1);
    return 0;
}
END
)

gcc -c map_reduce.c -I../include/
gcc -c tester.c -I../include
gcc map_reduce.o tester.o -o $BINARY_FILE

# test case 1
mkdir $TEMP_FOLDER
echo "1 2 3 1 2" > $TEMP_FOLDER/file1.txt
$BINARY_FILE $TEMP_FOLDER/file1.txt
echo ""

# test case 2
cp ../rsrc/stats_light/stats_light1.txt $TEMP_FOLDER/file2.txt
$BINARY_FILE $TEMP_FOLDER/file2.txt
echo ""
cat ../rsrc/stats_light/stats_light1.txt


# clean up
rm *.o
rm $BINARY_FILE
rm -rf $TEMP_FOLDER
# rm tester.c