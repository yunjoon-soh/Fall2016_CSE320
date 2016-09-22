#!/bin/sh
BIN=./bin/utf
OUTPUTDIR=./output

# 
OPTIONS=-v

# test files
TEST_16LE=./rsrc/trunk16le.txt
TEST_16BE=./rsrc/trunk16be.txt
TEST_8=./rsrc/trunk8.txt

TEST_OUTPUT=./test/out.txt

cd ../
make clean
make all

# test case 1 : LE->LE
echo *Running: $BIN $OPTIONS-u 16LE $TEST_16LE
$BIN $OPTIONS -u 16LE $TEST_16LE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *** Test Failed
	exit 1
fi

# test case 2 : LE->BE
echo *Running: $BIN $OPTIONS -u 16BE $TEST_16LE
$BIN $OPTIONS -u 16BE $TEST_16LE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *** Test Failed
	exit 1
fi

# test case 3 : BE->LE
echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
$BIN $OPTIONS -u 16LE $TEST_16BE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *** Test Failed
	exit 1
fi

# test case 4 : BE->BE
echo *Running: $BIN $OPTIONS -u 16BE $TEST_16BE
$BIN $OPTIONS -u 16BE $TEST_16BE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *** Test Failed
	exit 1
fi

############################################################################
# From utf8
# test case 5 : 8->LE
echo *Running: $BIN $OPTIONS-u 16LE $TEST_8
$BIN $OPTIONS -u 16LE $TEST_8 > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *** Test Failed
	exit 1
fi

# test case 6 : 8->BE
echo *Running: $BIN $OPTIONS -u 16BE $TEST_8
$BIN $OPTIONS -u 16BE $TEST_8 > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *** Test Failed
	exit 1
fi

############################################################################
# test case 7 : BE->8
echo *Running: $BIN $OPTIONS -u 8 $TEST_16BE
$BIN $OPTIONS -u 8 $TEST_16BE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_8
cmp $TEST_OUTPUT $TEST_8
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_8
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *** Test Failed
	exit 1
fi

# test case 8 : LE->8
echo *Running: $BIN $OPTIONS -u 8 $TEST_16LE
$BIN $OPTIONS -u 8 $TEST_16LE > $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_8
cmp $TEST_OUTPUT $TEST_8
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_8
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *** Test Failed
	exit 1
fi

# clean up
echo ***End of test cases, cleaning up
rm $TEST_OUTPUT
make clean
cd test

