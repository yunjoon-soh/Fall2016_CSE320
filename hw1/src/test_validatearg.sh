#!/bin/sh
BINARY_FILE="test.out"
TEMP_FOLDER="temp"

# cat ""
# (cat << EOF
# 	int main(int argc, char** argv) {
# 	    printf("Welcome to CSE 320!\n");
# 	    int validationValue = validateargs(argc, argv);
# 	    fprintf(stderr, "validationValue is %d\n", validationValue);
# 	    return 0;
# 	}
# EOF) >> tester.c

echo "#include \"main.h\"" > tester.c
echo "int main(int argc, char** argv) {" >> tester.c
echo "int validationValue = validateargs(argc, argv);" >> tester.c
echo "fprintf(stderr, \"validationValue is %d\", validationValue);}" >> tester.c

gcc -c functions.c
gcc -c tester.c
gcc functions.o tester.o -o $BINARY_FILE

# make valid directory
mkdir $TEMP_FOLDER

echo "Expected 0"
./$BINARY_FILE -h
echo ""
./$BINARY_FILE -h asdf
echo ""
./$BINARY_FILE -h ana $TEMP_FOLDER
echo ""
./$BINARY_FILE -h stats $TEMP_FOLDER
echo ""

echo " "
echo ""
echo "Expected 1"
./$BINARY_FILE ana $TEMP_FOLDER

echo " "
echo ""
echo "Expected 2"
./$BINARY_FILE stats $TEMP_FOLDER

echo " "
echo ""
echo "Expected 3"
./$BINARY_FILE -v ana $TEMP_FOLDER

echo " "
echo ""
echo "Expected 4"
./$BINARY_FILE -v stats $TEMP_FOLDER

echo " "
echo ""
echo "Expected -1"
./$BINARY_FILE -v -h ana $TEMP_FOLDER
echo ""
./$BINARY_FILE -h -v ana $TEMP_FOLDER
echo ""
./$BINARY_FILE -hv ana $TEMP_FOLDER
echo ""
./$BINARY_FILE -h -v ana $TEMP_FOLDER testsdfasdf
echo ""

# clean up
rm *.o
rm $BINARY_FILE
rmdir $TEMP_FOLDER
rm tester.c