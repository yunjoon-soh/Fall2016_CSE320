#!/bin/sh
# Check for number of parameters
if [ -z ${1} ]; then
	/bin/echo -e "\e[1;31mMissing parameter: nthreads\e[0m"
	exit
fi


make

# for each query
for QUERY in A B C D E; do
	/bin/echo -e "-----------------------------------------Query=$QUERY"
	# test if all 5 parts return the same
	for PART_NUM in 1 2 3 4 5; do
		./bin/lott $PART_NUM $QUERY $1 > tmp.out
		if [ $? -ne 0 ]; then
			/bin/echo -e "\x1B[1;31mError running $PART_NUM $QUERY $1 \x1B[0m"
		else
			/bin/echo -e "\x1B[1;32mSuccess running $PART_NUM $QUERY $1 \x1B[0m"
			tail -n +2 tmp.out > part$PART_NUM.out
		fi
	done

	# comparison starts
	for i in 1 2 3 4 5; do
		for j in 1 2 3 4 5; do
			diff part$i.out part$j.out
			if [ $? -ne 0 ]; then
				/bin/echo "\x1B[1;31mpart $i was different from part $j\x1B[0m"
				exit $?
			fi
		done
	done
	/bin/echo -e "\x1B[1;32mComparison Done!\a\x1B[0m"
done

rm -rf *.out
