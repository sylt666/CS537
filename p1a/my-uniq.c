#include<stdio.h>
#include<stdlib.h>
#include<string.h>
// #include<unistd.h>
// #include<sys/stat.h>
// #include<fcntl.h>

#define STDIN_FILE "-"

#define BUFFER_LEN 128

int main(int argc, char *argv[]) {
    // argv[0] is name of program you are running
    // gcc -o my-cat my-cat.c -Wall -Werror
    // ~cs537-1/tests/p1a/test-my-cat.csh

	FILE *fp = NULL;
	int ret = 0;

	if (argc < 2) {
        fp = stdin;
	}
    for (int i = 1; i < argc || fp; i++, fp = NULL) {
	     if (!fp) {
		     if (!strcmp(argv[i], STDIN_FILE)) {
			     fp = stdin;
		     } else {
			     fp = fopen(argv[i], "r");
		     }
	     }

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
                if (first == 1) {
                    if (strcmp(last_line, line) == 0) {
                    } else {
                    printf("%s", last_line);
                    }
                }
                last_line = strdup(line);
                first = 1;
            }

            if (strcmp(line, last_line) == 0) {
                printf("%s", line);
            }
            fclose(fp);
        }
    }
    return ret;
}
