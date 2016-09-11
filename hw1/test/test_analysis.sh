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
	struct Analysis test;
	
	FILE* f = fopen(argv[1], "r");

    int validationValue = analysis(f, &test, argv[1]);
    fprintf(stderr, "analysis value is %d\n", validationValue);

    analysis_print(test, 0, 1);
    return 0;
}
END
)

gcc -c map_reduce.c -I../include/
gcc -c tester.c
gcc map_reduce.o tester.o -o $BINARY_FILE

# test case 1
mkdir $TEMP_FOLDER
echo "abcaa" > $TEMP_FOLDER/file1.txt
$BINARY_FILE $TEMP_FOLDER/file1.txt
echo ""

# test case 2
cp ../rsrc/ana_light/ana_light1.txt $TEMP_FOLDER/file2.txt
$BINARY_FILE $TEMP_FOLDER/file2.txt
echo ""
cat ../rsrc/ana_light/ana_light1.txt


# clean up
rm *.o
rm $BINARY_FILE
rm -rf $TEMP_FOLDER
# rm tester.c