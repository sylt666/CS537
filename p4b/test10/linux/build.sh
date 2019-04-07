#!/usr/bin/env bash

gcc -c -o mapreduce.o mapreduce.c -Wall -Werror -pthread -O
gcc -o test test.c mapreduce.o -Wall -Werror