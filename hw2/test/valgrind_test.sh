#!/bin/sh
BIN=./bin/utf
OUTPUTDIR=./output

#
OPTIONS=

# test files
# TEST_16LE=./rsrc/LongLEBOM.txt
# TEST_16BE=./rsrc/LongBEBOM.txt
# TEST_8=./rsrc/Long8.txt

TEST_16LE=./rsrc/shotle.txt
TEST_16BE=./rsrc/shotbe.txt
TEST_8=./rsrc/shot8.txt
TEST_NOT_EXISTING=./asdfkaljsvcklajwe8y103.txt

TEST_OUTPUT=./test/out.txt

TIME=2

VALGRIND="valgrind --track-fds=yes "
cd ../
make clean
make all
clear

################################################################
################################################################
# argtest.sh

################################################################
# Cases expected to fail
################################################################
# clear
# # test case 0-1 : Help message along the argument
# echo *Running: $BIN -h
# echo *Expected: PRINT_USAGE and SUCCESS
# $VALGRIND $BIN -h
# sleep $TIME

# clear
# # test case 0-2 : Help message along the argument
# echo *Running: $BIN -h $TEST_16LE
# echo *Expected: PRINT_USAGE and SUCCESS
# $VALGRIND $BIN -h $TEST_16LE
# sleep $TIME

# clear
# # test case 0-3 : Help message along the argument
# echo *Running: $BIN -h -u 16LE $TEST_16LE
# echo *Expected: PRINT_USAGE and SUCCESS
# $VALGRIND $BIN -h -u 16LE $TEST_16LE
# sleep $TIME

# clear
# # test case 0-4 : Help message along the argument
# echo *Running: $BIN -hvv -u 16LE $TEST_16LE
# echo *Expected: PRINT_USAGE and SUCCESS
# $VALGRIND $BIN -hvv -u 16LE $TEST_16LE
# sleep $TIME

# clear
# # test case 0-5 : Help message along the argument
# echo *Running: $BIN -vv -u 16LE $TEST_16LE -h
# echo *Expected: PRINT_USAGE and SUCCESS
# $VALGRIND $BIN -vv -u 16LE $TEST_16LE -h
# sleep $TIME

# clear
# # test case 0-6 : Help message along the argument
# echo *Running: $BIN -v -u 16LE $TEST_16LE $TEST_OUTPUT -h
# $VALGRIND $BIN -v -u 16LE $TEST_16LE $TEST_OUTPUT -h
# sleep $TIME

################################################################
# Invalid args
################################################################
# clear
# # test case 1-1 : No mandatory arg
# echo ***Running: $BIN
# echo ***Expected: PRINT_USAGE and ERROR
# $VALGRIND $BIN
# RET=$?
# if [ $RET = 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi
# sleep $TIME

# clear
# # test case 1-2 : Yes mandatory arg with invalid args
# echo ***Running: $BIN -u 16LE $TEST_16LE -a
# echo ***Expected: PRINT_USAGE and ERROR
# $VALGRIND $BIN -u 16LE $TEST_16LE -a
# RET=$?
# if [ $RET = 0 ]; then # has to fail
# 	echo *Test failed... aborting
# 	exit 1
# fi
# sleep $TIME
################################################################
# No required argument
################################################################
# clear
# # test case 2-1 : NO_FILENAME_ERROR
# echo ***Running: $BIN -u 16LE
# echo ***Expected: NO_FILENAME_ERROR
# $VALGRIND $BIN -u 16LE
# RET=$?
# if [ $RET = 0 ]; then
# 	echo *Test failed... aborting
# 	exit 1
# fi
# sleep $TIME

# clear
# # test case 2-2 : NO_FILENAME_ERROR
# echo ***Running: $BIN --UTF=16LE
# echo ***Expected: NO_FILENAME_ERROR
# $VALGRIND $BIN --UTF=16LE
# RET=$?
# if [ $RET = 0 ]; then
# 	echo *Test failed... aborting
# 	exit 1
# fi
# sleep $TIME

################################################################
# Valid args but invalid files
################################################################
# clear
# # test case 3-1 : Input not existing
# rm -f $TEST_NOT_EXISTING
# echo ***Running: $BIN -u 16LE $TEST_NOT_EXISTING
# echo ***Expected: Abort
# $VALGRIND $BIN -u 16LE $TEST_NOT_EXISTING
# RET=$?
# if [ $RET = 0 ]; then
# 	echo *Test failed... aborting
# 	exit 1
# fi
# rm $TEST_NOT_EXISTING
# sleep $TIME

# clear
# # test case 3-2 : Output not existing (BE->LE)
# rm -f $TEST_NOT_EXISTING
# echo ***Running: $BIN -u 16LE $TEST_16BE $TEST_NOT_EXISTING
# echo ***Expected: Run properly and Write to stdout
# $VALGRIND $BIN -u 16LE $TEST_16BE $TEST_NOT_EXISTING
# RET=$?
# if [ $RET -ne 0 ]; then
# 	echo *Test failed... aborting
# 	exit 1
# fi
# rm $TEST_NOT_EXISTING
# sleep $TIME

# clear
# # test case 3-3 : Output not existing (LE->BE)
# rm -f $TEST_NOT_EXISTING
# echo ***Running: $BIN -u 16BE $TEST_16LE $TEST_NOT_EXISTING
# echo ***Expected: Run properly and Write to stdout
# $VALGRIND $BIN -u 16BE $TEST_16LE $TEST_NOT_EXISTING
# RET=$?
# if [ $RET -ne 0 ]; then
# 	echo *Test failed... aborting
# 	exit 1
# fi
# rm $TEST_NOT_EXISTING
# sleep $TIME

# clear
# # test case 3-4 : Input = Output
# rm -f $TEST_NOT_EXISTING
# cp $TEST_16LE $TEST_NOT_EXISTING
# echo ***Running: $BIN -u 16BE $TEST_NOT_EXISTING $TEST_NOT_EXISTING
# echo ***Expected: Input = Output, should fail
# $VALGRIND $BIN -u 16BE $TEST_NOT_EXISTING $TEST_NOT_EXISTING
# RET=$?
# if [ $RET = 0 ]; then
# 	echo *Test failed... aborting
# 	exit 1
# fi
# rm $TEST_NOT_EXISTING
# sleep $TIME
################################################################
################################################################
# stdout_test.sh

# # test case 1 : LE->LE
# echo *Running: $BIN $OPTIONS-u 16LE $TEST_16LE
# $VALGRIND $BIN $OPTIONS -u 16LE $TEST_16LE

# # test case 2 : LE->BE
# echo *Running: $BIN $OPTIONS -u 16BE $TEST_16LE
# $VALGRIND $BIN $OPTIONS -u 16BE $TEST_16LE

# # test case 3 : BE->LE
# echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 16LE $TEST_16BE

# # test case 4 : BE->BE
# echo *Running: $BIN $OPTIONS -u 16BE $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 16BE $TEST_16BE

# ############################################################################
# # From utf8
# # test case 5 : 8->LE
# echo *Running: $BIN $OPTIONS-u 16LE $TEST_8
# $VALGRIND $BIN $OPTIONS -u 16LE $TEST_8

# # test case 6 : 8->BE
# echo *Running: $BIN $OPTIONS -u 16BE $TEST_8
# $VALGRIND $BIN $OPTIONS -u 16BE $TEST_8

# ############################################################################
# # test case 7 : BE->8
# echo *Running: $BIN $OPTIONS -u 8 $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 8 $TEST_16BE

# # test case 8 : LE->8
# echo *Running: $BIN $OPTIONS -u 8 $TEST_16LE
# $VALGRIND $BIN $OPTIONS -u 8 $TEST_16LE
################################################################
################################################################
# outfile_test.sh

# # test case 3 : LE->LE
# echo *Running: $BIN $OPTIONS-u 16LE $TEST_16LE $TEST_OUTPUT
# $VALGRIND $BIN $OPTIONS -u 16LE $TEST_16LE $TEST_OUTPUT
# echo *Removed $TEST_OUTPUT
# rm $TEST_OUTPUT
# echo

# # test case 4 : LE->BE
# echo *Running: $BIN $OPTIONS -u 16BE $TEST_16LE
# $VALGRIND $BIN $OPTIONS -u 16BE $TEST_16LE $TEST_OUTPUT
# echo *Removed $TEST_OUTPUT
# rm $TEST_OUTPUT
# echo

# # test case 5 : BE->LE
# echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 16LE $TEST_16BE $TEST_OUTPUT
# echo *cmp $TEST_OUTPUT $TEST_16LE
# echo *Removed $TEST_OUTPUT
# rm $TEST_OUTPUT
# echo

# ##############################################################################
# # test case 6 : BE->BE
# echo *Running: $BIN $OPTIONS -u 16BE $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 16BE $TEST_16BE $TEST_OUTPUT
# echo *cmp $TEST_OUTPUT $TEST_16BE
# echo *Did not remove $TEST_OUTPUT
# echo

# # test for appending BE to file with BE BOM
# echo *Running: $BIN $OPTIONS -u 16BE $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 16BE $TEST_16BE $TEST_OUTPUT
# echo *Removed $TEST_OUTPUT 
# rm $TEST_OUTPUT
# echo

# ##############################################################################
# # test case 7: BE->LE
# echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 16LE $TEST_16BE $TEST_OUTPUT
# echo *Did not remove $TEST_OUTPUT
# echo

# # test for appending LE to file with LE BOM
# echo *Running: $BIN $OPTIONS -u 16LE $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 16LE $TEST_16BE $TEST_OUTPUT
# rm $TEST_OUTPUT
# echo

# ##############################################################################
# # test case 7: BE->8
# echo *Running: $BIN $OPTIONS -u 8 $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 8 $TEST_16BE $TEST_OUTPUT
# echo *Did not remove $TEST_OUTPUT
# echo

# # test for appending 8 to file with 8 BOM
# echo *Running: $BIN $OPTIONS -u 8 $TEST_16BE
# $VALGRIND $BIN $OPTIONS -u 8 $TEST_16BE $TEST_OUTPUT
# rm $TEST_OUTPUT
# echo

############################################
############################################
# vtest.sh

# test case 1 : LE->LE
echo ***Running: $BIN -vvu 16LE $TEST_16LE
$VALGRIND $BIN -vvu 16LE $TEST_16LE

# test case 2 : LE->BE
echo ***Running: $BIN -vvu 16BE $TEST_16LE
$VALGRIND $BIN -vvu 16BE $TEST_16LE

# test case 3 : BE->LE
echo ***Running: $BIN -vvu 16LE $TEST_16BE
$VALGRIND $BIN -vvu 16LE $TEST_16BE > $TEST_OUTPUT

# test case 4 : BE->BE
echo ***Running: $BIN -vvu 16BE $TEST_16BE
$VALGRIND $BIN -vvu 16BE $TEST_16BE > $TEST_OUTPUT


############################################
# UTF8 OUTPUT

# test case 5 : BE->8
rm $TEST_OUTPUT
echo ***Running: $BIN $OPTIONS -vvu 8 $TEST_16BE
$VALGRIND $BIN $OPTIONS -vvu 8 $TEST_16BE $TEST_OUTPUT

# test case 6 : LE->8
rm $TEST_OUTPUT
echo ***Running: $BIN $OPTIONS -vvu 8 $TEST_16LE
$VALGRIND $BIN $OPTIONS -vvu 8 $TEST_16LE $TEST_OUTPUT

#########################################3
# UTF8 INPUT

# test case 7 : 8->BE
rm $TEST_OUTPUT
echo ***Running: $BIN -vvu 16BE $TEST_8 $TEST_OUTPUT
$VALGRIND $BIN -vvu 16BE $TEST_8 $TEST_OUTPUT

# test case 8 : 8->LE
rm $TEST_OUTPUT
echo ***Running: $BIN -vvu 16LE $TEST_8 $TEST_OUTPUT
$VALGRIND $BIN -vvu 16LE $TEST_8 $TEST_OUTPUT

############################################

# test case 9 : 8->8
rm $TEST_OUTPUT
echo ***Running: $BIN -vvu 8 $TEST_8 $TEST_OUTPUT
$VALGRIND $BIN -vvu 8 $TEST_8 $TEST_OUTPUT

################################################################
# clean up
################################################################
echo ***End of test cases, cleaning up
rm -f $TEST_OUTPUT
make clean
cd test
