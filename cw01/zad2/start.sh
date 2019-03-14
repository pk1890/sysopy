#!/bin/bash

S="search_directory"
D="delete_block"
T="stress_test"
DIR="/"
FILE="Makefile"
TMP="/tmp/tmp"
ARGS=" 100 $T $DIR $FILE $TMP 500000 $D 0"
export LD_LIBRARY_PATH=/home/mleko/sysopy/cw01/zad1:$LD_LIBRARY_PATH

echo "=====BIG SEARCH:"

./zad2_static 100 $S "/" "\"*\"" $TMP $D 0

echo "=====MEDIUM SEARCH:"

./zad2_static 100 $S "/usr" "\"*\"" $TMP $D 0
echo "=====SMALL SEARCH:"

./zad2_static 100 $S "/usr/bin" "\"*\"" $TMP $D 0
echo "=================STRESS TEST"
./zad2_static $ARGS
