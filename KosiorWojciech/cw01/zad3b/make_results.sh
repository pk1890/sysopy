#!/bin/bash

cat comments.txt > results3b.txt

for OPTIM in 0 1 2 3 s fast g; do

    make clean
    make all OPTIM=$OPTIM

    echo >> results3b.txt
    echo Optimization -O$OPTIM >> results3b.txt
    echo sizes: >> results3b.txt
    ls -sh program_{static,shared,dynamic} >> results3b.txt
    echo >> results3b.txt
    
    for LOG in logfile_{static,shared,dynamic}_O$OPTIM; do
        echo ${LOG} >> results3b.txt
        cat ${LOG} >> results3b.txt
        echo >> results3b.txt
    done
    
done

make clean
