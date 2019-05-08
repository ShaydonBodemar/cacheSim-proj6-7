#!/bin/bash
export compile="gcc -std=c99 -Wall -c -g -fpic"
export LD_LIBRARY_PATH=`pwd`

$(compile) -o objs/storage.o           storage.c
$(compile) -o objs/trace.o             trace.c
$(compile) -o objs/memory.o            memory.c
$(compile) -o objs/cache_direct.o      cache_direct.c
$(compile) -o objs/cache_associative_full.o cache_associative_full.c
$(compile) -o objs/cache_associative_nway.o cache_associative_nway.c

gcc -shared -o libs/libshared.so objs/*

gcc -std=c99 -Wall -g  \
    -o test1 test1.c \
   -L$(LD_LIBRARY_PATH) -lshared

gcc -std=c99 -Wall -g  \
    -o test1a test1a.c \
    -L$(LD_LIBRARY_PATH) -lshared

gcc -std=c99 -Wall -g \
    -o test2 test2.c \
    -L$(LD_LIBRARY_PATH) -lshared

gcc -std=c99 -Wall -g \
    -o test3 test3.c \
    -L$(LD_LIBRARY_PATH) -lshared

gcc -std=c99 -Wall -g \
    -o test_matrix_sum_16x16 test_matrix_sum_16x16.c \
    -L$(LD_LIBRARY_PATH) -lshared

gcc -std=c99 -Wall -g \
    -o test_matrix_sum_8x4 test_matrix_sum_8x4.c \
    -L$(LD_LIBRARY_PATH) -lshared

gcc -std=c99 -Wall -g \
    -o test_transpose_8x8 test_transpose_8x8.c \
    -L$(LD_LIBRARY_PATH) -lshared

gcc -std=c99 -Wall -g \
    -o test_transpose_8x8_improved test_transpose_8x8_improved.c \
    -L$(LD_LIBRARY_PATH) -lshared