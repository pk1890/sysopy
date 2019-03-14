#!/bin/bash

FILE=results3b.txt
echo "" > $FILE
function test_option {
	echo "" >> $FILE
	echo "============================================================= TESTY OPTION $1 =======================================================" >> $FILE 
	echo "" >> $FILE
	echo "===================================================TESTY OPTION $1"
	echo "COMPILING... "
	make clean
	make all OPT=$1
	echo "RUNNING MAIN_STATIC"
	./start_single_test.sh main_static 2> /dev/null >> $FILE
	echo "RUNNING MAIN_SHARED"
	./start_single_test.sh main_shared 2> /dev/null >> $FILE
	echo "RUNNING MAIN_DYNAMIC"
	./start_single_test.sh main_dynamic 2> /dev/null >> $FILE
	echo "FINISHED"
}

test_option O0
test_option O1
test_option O2
test_option O3
test_option Os
test_option Og
