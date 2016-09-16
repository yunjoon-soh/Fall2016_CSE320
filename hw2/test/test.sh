#!/bin/sh
BIN=./bin/utfconverter
OUTPUTDIR=./output

# test files
TEST_16BE_Special=./rsrc/utf16BE-special.txt
TEST_16LE=./rsrc/utf16le.txt

cd ../
make clean
make all
echo Running: $BIN $TEST_16LE
$BIN $TEST_16LE

# test case 2
echo Running: $BIN $TEST_16LE -u 16LE
echo Expected: NO_FILENAME_ERROR
$BIN $TEST_16LE -u 16LE

# test case 3
echo Running: $BIN -u 16LE $TEST_16LE
$BIN -u 16LE $TEST_16LE

# clean up
#make clean
cd test

