#!/bin/bash

LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:../zad1 \
./program \
    create_array 40 \
    search_directory / index.html tmpfile1 \
    search_directory /usr/share/kbd/ LC_MESSAGES tmpfile2 \
    search_directory .. Makefile tmpfile3 \
    load_tmpfile ./tmpfile1 \
    load_tmpfile ./tmpfile2 \
    load_tmpfile ./tmpfile3 \
    remove_block 39 \
    remove_block 38 \
    remove_block 37 \
    load_tmpfile ./tmpfile3 \
    load_tmpfile ./tmpfile2 \
    load_tmpfile ./tmpfile1 \
    remove_block 37 \
    remove_block 38 \
    remove_block 39 \
    free_array \
    create_array 10 \
    search_and_load /usr bin tmpfile4 \
    search_and_load /gnu/store bin tmpfile5 \
    search_and_load /gnu Makefile tmpfile6 \
    remove_block 7 \
    remove_block 9 \
    remove_block 8

rm tmpfile[1-6]
