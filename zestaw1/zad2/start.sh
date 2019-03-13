#!/bin/bash

S="search_directory"
D="delete_block"
T="stress_test"
DIR="/home"
FILE="Makefile"
TMP="tmp"
ARGS=" 100 $S $DIR $FILE $TMP $D 0"
export LD_LIBRARY_PATH=/home/mleko/sysopy/zestaw1/zadania:$LD_LIBRARY_PATH
./main_shared
