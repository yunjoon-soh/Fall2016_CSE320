#!/bin/sh

make clean && make && clear

COUNT=0
# for different subset of data
for i in b c d e f g h i j k l m n o p q r s t u v w x y z; do
	/bin/echo -e "\e[34mCreate subset of data/$i*...\e[0m"
	rm -rf data/
	mkdir data/
	cp data_all/$i* data/

	# Run test with different number of threads
	for j in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 27 31 39 41 47 100; do
		/bin/echo "-------------------------------------------$COUNT"
		./test/test.sh $j
		COUNT=$((COUNT+1))
		if [ $? -ne 0 ]; then
			break;
		fi
	done
done

