#!/bin/sh
if [ -z ${1} ]; then
	/bin/echo -e "\e[1;31mMissing parameter: nthreads\e[0m"
	exit
fi

# test if all 5 parts return the same
for PART_NUM in 1 2 3 4 5; do
	valgrind ./bin/lott $PART_NUM A $1 > tmp.out
	if [ $? -ne 0 ]; then
		/bin/echo -e "\x1B[1;31mError running $PART_NUM A $1 \x1B[0m"
	else
		/bin/echo -e "\x1B[1;32mSuccess running $PART_NUM A $1 \x1B[0m"
		tail -n +2 tmp.out > part$PART_NUM.out
	fi
done

rm -rf *.out
