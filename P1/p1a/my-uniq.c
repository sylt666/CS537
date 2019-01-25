#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
    // argv[0] is name of program you are running
    // gcc -o my-cat my-cat.c -Wall -Werror
    // ~cs537-1/tests/p1a/test-my-cat.csh
    // ~cs537-1/tests/p1a/test-my-cat.csh -v
    // ./my-cat [file]

    FILE *f = fopen(argv[1], "r");

    // if error exit
    if (f == NULL) {
        printf("file %s not found\n", argv[1]);
        exit(1);
    }
    
    // Passing NULL into fclose() causes segmentation fault when file does not exist
    fclose(f);

    return 0;
}