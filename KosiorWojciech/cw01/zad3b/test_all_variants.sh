#!/bin/bash

for OPTIM in 0 1 2 3 s fast g; do

    make clean
    make test OPTIM=$OPTIM

    for LOG in logfile_{static,shared,dynamic}; do
        mv ${LOG} ${LOG}_O$OPTIM
    done
    
done

make clean
