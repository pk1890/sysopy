#!/bin/bash

make clean
make test

cat comments.txt > results3a.txt
echo >> results3a.txt
echo sizes: >> results3a.txt
ls -sh program_{static,shared,dynamic} >> results3a.txt
echo >> results3a.txt
    
for LOG in logfile_{static,shared,dynamic}; do
    echo ${LOG} >> results3a.txt
    cat ${LOG} >> results3a.txt
    echo >> results3a.txt
done

make clean
