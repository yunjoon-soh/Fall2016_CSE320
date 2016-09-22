#!/bin/sh
BIN=./bin/utf
OUTPUTDIR=./output

# test files
TEST_16BE_Special=./rsrc/utf16BE-special.txt
TEST_16LE_Special=./rsrc/utf16LE-special.txt

# TEST_16LE=./rsrc/LongLEBOM.txt
# TEST_16BE=./rsrc/LongBEBOM.txt
# TEST_8=./rsrc/Long8.txt

TEST_16LE=./rsrc/shotle.txt
TEST_16BE=./rsrc/shotbe.txt
TEST_8=./rsrc/shot8.txt

TEST_OUTPUT=./test/out.txt

cd ../
make clean
make all
clear


# test case 1 : LE->LE
echo ***Running: $BIN -vvu 16LE $TEST_16LE
$BIN -vvu 16LE $TEST_16LE > $TEST_OUTPUT
echo ***cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi

# test case 2 : LE->BE
echo ***Running: $BIN -vvu 16BE $TEST_16LE
$BIN -vvu 16BE $TEST_16LE > $TEST_OUTPUT
echo ***cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
fi

# test case 3 : BE->LE
echo ***Running: $BIN -vvu 16LE $TEST_16BE
$BIN -vvu 16LE $TEST_16BE > $TEST_OUTPUT
echo ***cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
fi

# test case 4 : BE->BE
echo ***Running: $BIN -vvu 16BE $TEST_16BE
$BIN -vvu 16BE $TEST_16BE > $TEST_OUTPUT
echo ***cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
fi


#########################################3
# UTF8 OUTPUT

# test case 5 : BE->8
rm $TEST_OUTPUT
echo ***Running: $BIN $OPTIONS -vvu 8 $TEST_16BE
$BIN $OPTIONS -vvu 8 $TEST_16BE $TEST_OUTPUT
echo ***cmp $TEST_OUTPUT $TEST_8
cmp $TEST_OUTPUT $TEST_8
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_8
	echo Acutal:
	xxd $TEST_OUTPUT
	exit 1
fi

# test case 6 : LE->8
rm $TEST_OUTPUT
echo ***Running: $BIN $OPTIONS -vvu 8 $TEST_16LE
$BIN $OPTIONS -vvu 8 $TEST_16LE $TEST_OUTPUT
echo ***cmp $TEST_OUTPUT $TEST_8
cmp $TEST_OUTPUT $TEST_8
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_8
	echo Acutal:
	xxd $TEST_OUTPUT
	exit 1
fi

#########################################3
# UTF8 INPUT

test case 7 : 8->BE
rm $TEST_OUTPUT
echo ***Running: $BIN -vvu 16BE $TEST_8 $TEST_OUTPUT
$BIN -vvu 16BE $TEST_8 $TEST_OUTPUT
echo ***cmp $TEST_OUTPUT $TEST_16BE
cmp $TEST_OUTPUT $TEST_16BE
RET=$?
if [ $RET -ne 0 ]; then
	echo Input:
	xxd $TEST_8
	echo Expected:
	xxd $TEST_16BE
	echo Acutal:
	xxd $TEST_OUTPUT
	exit 1
fi

# test case 8 : 8->LE
rm $TEST_OUTPUT
echo ***Running: $BIN -vvu 16LE $TEST_8 $TEST_OUTPUT
$BIN -vvu 16LE $TEST_8 $TEST_OUTPUT
echo ***cmp $TEST_OUTPUT $TEST_16LE
cmp $TEST_OUTPUT $TEST_16LE
RET=$?
if [ $RET -ne 0 ]; then
	echo Expected:
	xxd $TEST_16LE
	echo Acutal:
	xxd $TEST_OUTPUT
	exit 1
fi

# clean up
make clean
cd test

