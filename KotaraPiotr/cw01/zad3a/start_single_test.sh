#!/bin/bash

S="search_directory"
D="delete_block"
T="stress_test"
DIR="/"
FILE="Makefile"
TMP="/tmp/tmp"
ARGS=" 100 $T $DIR $FILE $TMP 500000 $D 0"
export LD_LIBRARY_PATH=/home/mleko/sysopy/cw01/zad1:$LD_LIBRARY_PATH
echo ""
echo ""
echo "TESTING $1"

echo "=====BIG SEARCH:"

./$1 100 $S "/" "\"*\"" $TMP $D 0

echo "=====MEDIUM SEARCH:"

./$1 100 $S "/usr" "\"*\"" $TMP $D 0
echo "=====SMALL SEARCH:"

./$1 100 $S "/usr/bin" "\"*\"" $TMP $D 0
echo "=================STRESS TEST"
./$1 $ARGS
