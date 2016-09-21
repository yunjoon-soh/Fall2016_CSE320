#!/bin/sh
BIN=./bin/utf
OUTPUTDIR=./output

# 
OPTIONS=-vv

# test files
TEST_16BE_Special=./rsrc/utf16BE-special.txt
TEST_16LE_Special=./rsrc/utf16LE-special.txt
TEST_16LE=./rsrc/shotle.txt
TEST_16BE=./rsrc/shotbe.txt

TEST_OUTPUT=./test/out.txt

cd ../
make clean
make all

rm $TEST_OUTPUT
# test case 3 : LE->LE
echo *Running: $BIN $OPTIONS-u 16LE $TEST_16LE $TEST_OUTPUT
$BIN $OPTIONS -u 16LE $TEST_16LE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
echo *Removed $TEST_OUTPUT
rm $TEST_OUTPUT
echo

# test case 4 : LE->BE
echo *Running: $BIN $OPTIONS -u 16BE $TEST_16LE
$BIN $OPTIONS -u 16BE $TEST_16LE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
echo *Removed $TEST_OUTPUT
rm $TEST_OUTPUT
echo

# test case 5 : BE->LE
echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
$BIN $OPTIONS -u 16LE $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
echo *Removed $TEST_OUTPUT
rm $TEST_OUTPUT
echo

# test case 6 : BE->BE
echo *Running: $BIN $OPTIONS -u 16BE $TEST_16BE
$BIN $OPTIONS -u 16BE $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
echo *Did not remove $TEST_OUTPUT
echo

echo *Running: $BIN $OPTIONS -u 16BE $TEST_16BE
$BIN $OPTIONS -u 16BE $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
echo *Removed $TEST_OUTPUT
rm $TEST_OUTPUT
echo

# test case 7: BE->LE
echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
$BIN $OPTIONS -u 16LE $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
echo *Did not remove $TEST_OUTPUT
echo

echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
$BIN $OPTIONS -u 16LE $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
echo

# clean up
#make clean
cd test

