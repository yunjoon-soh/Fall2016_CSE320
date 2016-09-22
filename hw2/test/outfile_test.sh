#!/bin/sh
BIN=./bin/utf
OUTPUTDIR=./output

# 
OPTIONS=-vv

# test files
TEST_16LE=./rsrc/shotle.txt
TEST_16BE=./rsrc/shotbe.txt
TEST_8=./rsrc/shot8.txt

TEST_OUTPUT=./test/out.txt

cd ../
make clean
make all

rm $TEST_OUTPUT
##############################################################################
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
	echo *Test failed... aborting
	exit 1
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
	echo *Test failed... aborting
	exit 1
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
	echo *Test failed... aborting
	exit 1
fi
echo *Removed $TEST_OUTPUT
rm $TEST_OUTPUT
echo

##############################################################################
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
	echo *Test failed... aborting
	exit 1
fi
echo *Did not remove $TEST_OUTPUT
echo

# test for appending BE to file with BE BOM
echo *Running: $BIN $OPTIONS -u 16BE $TEST_16BE
$BIN $OPTIONS -u 16BE $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected twice print of this:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
echo *Removed $TEST_OUTPUT 
rm $TEST_OUTPUT
echo

##############################################################################
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
	echo *Test failed... aborting
	exit 1
fi
echo *Did not remove $TEST_OUTPUT
echo

# test for appending LE to file with LE BOM
echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
$BIN $OPTIONS -u 16LE $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected twice print of this:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi
rm $TEST_OUTPUT
echo

##############################################################################
# test case 7: BE->8
echo *Running: $BIN $OPTIONS -u 8 $TEST_16BE
$BIN $OPTIONS -u 8 $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_8
cmp $TEST_OUTPUT $TEST_8
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_8
	echo Acutal:
	xxd $TEST_OUTPUT
	echo *Test failed... aborting
	exit 1
fi
echo *Did not remove $TEST_OUTPUT
echo

# test for appending 8 to file with 8 BOM
echo *Running: $BIN $OPTIONS -u 8 $TEST_16BE
$BIN $OPTIONS -u 8 $TEST_16BE $TEST_OUTPUT
echo *cmp $TEST_OUTPUT $TEST_8
cmp $TEST_OUTPUT $TEST_8
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected twice print of this:
	xxd $TEST_8
	echo Acutal:
	xxd $TEST_OUTPUT
fi
rm $TEST_OUTPUT
echo

# clean up
#make clean
cd test

