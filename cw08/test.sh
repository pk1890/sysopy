#!/bin/bash

test_thread() {
	echo "=="
	echo "== $2 threads =="
	./filtering lena.ascii.pgm gaussian_blur.fil blurred_lena.pgm $2 $1
}

test_variant() {
	echo "==="
	echo "=== $1 ==="
	test_thread "$1" 1
	test_thread "$1" 2
	test_thread "$1" 4
	test_thread "$1" 8
}

test_variant "B"
test_variant "I"
