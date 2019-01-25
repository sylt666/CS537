#! /bin/csh -f
set TEST_HOME = ~CS537/P1/p1a
set source_file = my-sed.c
set binary_file = my-sed
set bin_dir = ${TEST_HOME}/bin
set test_dir = ${TEST_HOME}/tests-my-sed
${bin_dir}/generic-tester.py -s $source_file -b $binary_file -t $test_dir $argv[*]
