#!/bin/bash
    

FILE=/tmp/tmpfile
COPY1=/tmp/tmpfile1
COPY2=/tmp/tmpfile2

    

function testf { #1st arg - blocks_no 2nd - char in block
    echo  "" >> wyniki.txt
    echo "Testing $1 blocks $2 bytes each ================================================" >> wyniki.txt
    echo  "" >> wyniki.txt

    ./zad1 gen \
        $FILE \
        $1 \
        $2 \
        copyL \
        $FILE \
        $COPY1 \
        $1 \
        $2 \
        copyS \
        $FILE \
        $COPY2 \
        $1 \
        $2 \
        sortL \
        $COPY1 \
        $1 \
        $2 \
        sortS \
        $COPY2 \
        $1 \
        $2 >> wyniki.txt


}
rm wyniki.txt
echo "testing 4096 blocks"
testf 4096 1 
testf 4096 2 
testf 4096 512
testf 4096 1024
testf 4096 2048
testf 4096 4096
testf 4096 8192
echo "testing 512 blocks"
testf 512 1 
testf 512 2 
testf 512 512
testf 512 1024
testf 512 2048
testf 512 512
testf 512 8192