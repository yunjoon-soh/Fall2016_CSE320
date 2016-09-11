#!/bin/sh
#test_total.sh
BINARY_FILE="map_reduce"
TEMP_FOLDER="temp"

gcc -c map_reduce.c -I../include/
gcc -c tester.c -I../include
gcc map_reduce.o tester.o -o $BINARY_FILE

# test case #1
echo "* Expected: Print usage and exit"
$BINARY_FILE -h ana $TEMP_FOLDER

test case #2
echo "* Expected: Print usage and exit(no such folder)"
$BINARY_FILE ana $TEMP_FOLDER

# test case #3
mkdir $TEMP_FOLDER
cp ../rsrc/ana_light/* ./$TEMP_FOLDER/
echo "* Expected: Run properly (ana)"
$BINARY_FILE ana $TEMP_FOLDER
rm -rf $TEMP_FOLDER # clean

# test case #4
mkdir $TEMP_FOLDER
cp ../rsrc/ana_light/* ./$TEMP_FOLDER/
echo "* Expected: Run properly (-v ana)"
$BINARY_FILE -v ana $TEMP_FOLDER
rm -rf $TEMP_FOLDER # clean

# test case #5
mkdir $TEMP_FOLDER
cp ../rsrc/stats_light/* ./$TEMP_FOLDER/
echo "* Expected: Run properly (stats)"
$BINARY_FILE stats $TEMP_FOLDER
rm -rf $TEMP_FOLDER # clean

# test case #5
mkdir $TEMP_FOLDER
cp ../rsrc/stats_light/* ./$TEMP_FOLDER/
echo "* Expected: Run properly (-v stats)"
$BINARY_FILE -v stats $TEMP_FOLDER
rm -rf $TEMP_FOLDER # clean

# clean up
rm *.o
rm $BINARY_FILE
rm -rf $TEMP_FOLDER