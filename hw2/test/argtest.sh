#!/bin/sh
BIN=./bin/utf
OUTPUTDIR=./output

# test files
TEST_16BE_Special=./rsrc/utf16BE-special.txt
TEST_16LE_Special=./rsrc/utf16LE-special.txt

TEST_16LE=./rsrc/shotle.txt
TEST_16BE=./rsrc/shotbe.txt

TEST_OUTPUT=./test/out.txt

TEST_NOT_EXISTING=./asdkljfklejnv_lqkwejrlkjasd.txt

cd ../
make clean
make all
clear

################################################################
# Cases expected to fail
################################################################
# test case 0-1 : Help message along the argument
# echo *Running: $BIN -h
# echo *Expected: PRINT_USAGE and SUCCESS
# $BIN -h
# RET=$?
# if [ $RET -ne 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi

# # test case 0-2 : Help message along the argument
# echo *Running: $BIN -h $TEST_16LE
# echo *Expected: PRINT_USAGE and SUCCESS
# $BIN -h $TEST_16LE
# RET=$?
# if [ $RET -ne 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi

# # test case 0-3 : Help message along the argument
# echo *Running: $BIN -h -u 16LE $TEST_16LE
# echo *Expected: PRINT_USAGE and SUCCESS
# $BIN -h -u 16LE $TEST_16LE
# RET=$?
# if [ $RET -ne 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi

# # test case 0-4 : Help message along the argument
# echo *Running: $BIN -hvv -u 16LE $TEST_16LE
# echo *Expected: PRINT_USAGE and SUCCESS
# $BIN -hvv -u 16LE $TEST_16LE
# RET=$?
# if [ $RET -ne 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi

# # test case 0-5 : Help message along the argument
# echo *Running: $BIN -vv -u 16LE $TEST_16LE -h
# echo *Expected: PRINT_USAGE and SUCCESS
# $BIN -vv -u 16LE $TEST_16LE -h
# RET=$?
# if [ $RET -ne 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi

# # test case 0-6 : Help message along the argument
# echo *Running: $BIN -v -u 16LE $TEST_16LE $TEST_OUTPUT -h
# echo *Expected: PRINT_USAGE and SUCCESS
# $BIN -v -u 16LE $TEST_16LE $TEST_OUTPUT -h
# RET=$?
# if [ $RET -ne 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi

################################################################
# Invalid args
################################################################
# # test case 1-1 : No mandatory arg
# echo ***Running: $BIN
# echo ***Expected: PRINT_USAGE and ERROR
# $BIN
# RET=$?
# if [ $RET = 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi

# # test case 1-2 : Yes mandatory arg with invalid args
# echo ***Running: $BIN -u 16LE $TEST_16LE -a
# echo ***Expected: PRINT_USAGE and ERROR
# $BIN -u 16LE $TEST_16LE -a
# RET=$?
# if [ $RET = 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi

################################################################
# No required argument
################################################################
# # test case 2-1 : NO_FILENAME_ERROR
# echo ***Running: $BIN -u 16LE
# echo ***Expected: NO_FILENAME_ERROR
# $BIN -u 16LE
# RET=$?
# if [ $RET = 0 ]; then
# 	echo *Test failed... aborting
# 	exit 1
# fi

# # test case 2-2 : NO_FILENAME_ERROR
# echo ***Running: $BIN --UTF=16LE
# echo ***Expected: NO_FILENAME_ERROR
# $BIN --UTF=16LE
# RET=$?
# if [ $RET = 0 ]; then
# 	echo *Test failed... aborting
# 	exit 1
# fi

################################################################
# Valid args but invalid files
################################################################
# test case 3-1 : Input not existing
rm -f $TEST_NOT_EXISTING
echo ***Running: $BIN -u 16LE $TEST_NOT_EXISTING
echo ***Expected: Abort
$BIN -u 16LE $TEST_NOT_EXISTING
RET=$?
if [ $RET = 0 ]; then
	echo *Test failed... aborting
	exit 1
fi
rm $TEST_NOT_EXISTING

# test case 3-2 : Output not existing (BE->LE)
rm -f $TEST_NOT_EXISTING
echo ***Running: $BIN -u 16LE $TEST_16BE $TEST_NOT_EXISTING
echo ***Expected: Run properly and Write to stdout
$BIN -u 16LE $TEST_16BE $TEST_NOT_EXISTING
RET=$?
if [ $RET -ne 0 ]; then
	echo *Test failed... aborting
	exit 1
fi
echo ***cmp $TEST_16LE $TEST_NOT_EXISTING
cmp $TEST_16LE $TEST_NOT_EXISTING
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT

	rm $TEST_NOT_EXISTING
	exit 1
fi
rm $TEST_NOT_EXISTING

# test case 3-3 : Output not existing (LE->BE)
rm -f $TEST_NOT_EXISTING
echo ***Running: $BIN -u 16BE $TEST_16LE $TEST_NOT_EXISTING
echo ***Expected: Run properly and Write to stdout
$BIN -u 16BE $TEST_16LE $TEST_NOT_EXISTING
RET=$?
if [ $RET -ne 0 ]; then
	echo *Test failed... aborting
	exit 1
fi
echo ***cmp $TEST_16BE $TEST_NOT_EXISTING
cmp $TEST_16BE $TEST_NOT_EXISTING
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT

	rm $TEST_NOT_EXISTING
	exit 1
fi
rm $TEST_NOT_EXISTING

# test case 3-4 : Input = Output
rm -f $TEST_NOT_EXISTING
cp $TEST_16LE $TEST_NOT_EXISTING
echo ***Running: $BIN -u 16BE $TEST_NOT_EXISTING $TEST_NOT_EXISTING
echo ***Expected: Input = Output, should fail
$BIN -u 16BE $TEST_NOT_EXISTING $TEST_NOT_EXISTING
RET=$?
if [ $RET = 0 ]; then
	echo *Test failed... aborting
	exit 1
fi
rm $TEST_NOT_EXISTING

# # test case 4 : LE->BE
# echo *Running: $BIN $OPTIONS -u 16BE $TEST_16LE
# $BIN $OPTIONS -u 16BE $TEST_16LE > $TEST_OUTPUT
# echo *cmp $TEST_OUTPUT $TEST_16BE
# cmp $TEST_OUTPUT $TEST_16BE
# RET=$?
# if [ $RET -ne 0 ]; then
# 	echo Expected:
# 	xxd $TEST_16BE
# 	echo Acutal:
# 	xxd $TEST_OUTPUT
# 	exit 1
# fi

# # test case 5 : BE->LE
# echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
# $BIN $OPTIONS -u 16LE $TEST_16BE > $TEST_OUTPUT
# echo *cmp $TEST_OUTPUT $TEST_16LE
# cmp $TEST_OUTPUT $TEST_16LE
# RET=$?
# if [ $RET -ne 0 ]; then
# 	echo Expected:
# 	xxd $TEST_16LE
# 	echo Acutal:
# 	xxd $TEST_OUTPUT
# 	exit 1
# fi

# # test case 6 : BE->BE
# echo *Running: $BIN $OPTIONS -u 16BE $TEST_16BE
# $BIN $OPTIONS -u 16BE $TEST_16BE > $TEST_OUTPUT
# echo *cmp $TEST_OUTPUT $TEST_16BE
# cmp $TEST_OUTPUT $TEST_16BE
# RET=$?
# if [ $RET -ne 0 ]; then
# 	echo Expected:
# 	xxd $TEST_16BE
# 	echo Acutal:
# 	xxd $TEST_OUTPUT
# 	exit 1
# fi

# clean up
echo ***End of test cases, cleaning up
make clean
cd test

