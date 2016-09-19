#!/bin/sh
BIN=./bin/utf
OUTPUTDIR=./output

# test files
TEST_16BE_Special=./rsrc/utf16BE-special.txt
TEST_16LE_Special=./rsrc/utf16LE-special.txt
TEST_16LE=./rsrc/utf16le.txt
TEST_16BE=./rsrc/16be.txt

TEST_OUTPUT=./test/out.txt

cd ../
make clean
make all
echo *Running: $BIN $TEST_16LE
$BIN $TEST_16LE

# test case 2
echo *Running: $BIN $TEST_16LE -u 16LE
echo *Expected: NO_FILENAME_ERROR
$BIN $TEST_16LE -u 16LE

# test case 3 : LE->LE
echo *Running: $BIN -u 16LE $TEST_16LE
$BIN -u 16LE $TEST_16LE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi

# test case 4 : LE->BE
echo *Running: $BIN -u 16BE $TEST_16LE
$BIN -u 16BE $TEST_16LE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
fi

# test case 5 : BE->LE
echo *Running: $BIN -u 16LE $TEST_16BE
$BIN -u 16LE $TEST_16BE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi

# test case 6 : BE->BE
echo *Running: $BIN -u 16BE $TEST_16BE
$BIN -u 16BE $TEST_16BE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
fi

# clean up
#make clean
cd test

