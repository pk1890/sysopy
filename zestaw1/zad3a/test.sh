#!/bin/bash

FILE=results3a.txt

echo "TESTY" > $FILE 

echo "RUNNING MAIN_STATIC"
./start_single_test.sh main_static 2> /dev/null >> $FILE
echo "RUNNING MAIN_SHARED"
./start_single_test.sh main_shared 2> /dev/null >> $FILE
echo "RUNNING MAIN_DYNAMIC"
./start_single_test.sh main_dynamic 2> /dev/null >> $FILE
echo "FINISHED"
