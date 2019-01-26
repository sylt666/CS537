#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char *argv[]) {
    // argv[0] is name of program you are running
    // gcc -o my-cat my-cat.c -Wall -Werror
    // ~cs537-1/tests/p1a/test-my-cat.csh
    // ~cs537-1/tests/p1a/test-my-cat.csh -v
    // ./my-cat [file]
	
	if (argc < 2) {
        printf("my-uniq: [file …]\n");
		exit(1);
	}
	else {
            for (int i = 1; i < argc; i++) {
			    FILE *fp = fopen(argv[i], "r");
			    if (fp == NULL) {
				    printf("my-uniq: cannot open file\n");
				    exit(1);
			    } else {
                    char* last_line = NULL;
                    char* line = NULL;
                    size_t length = 0;
                    ssize_t read = 0;
                    int first = 0;
                    while ((read = getline(&line, &length, fp) != -1)) {
                        //printf("last_line = %s\nline = %s\n", last_line, line);
                        if (first == 1) {
                            if (strcmp(last_line, line) == 0) {
                                //printf("%s = %s\n", last_line, line);
                            } else {
                                printf("%s\n", last_line);
                            }
                        }
                        //printf("setting last_line: %s TO line: %s\n", last_line, line);
                        last_line = strdup(line);
                        first = 1;
                    }
                    fclose(fp);
			    }
            }
		}	
	return 0;
}