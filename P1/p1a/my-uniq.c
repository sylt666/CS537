#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define STDIN_FILE "-"

#define BUFFER_LEN 128

char *getln(FILE *file) {
	char *ret = NULL, line[BUFFER_LEN];
	int len = 0;

	while(fgets(line, BUFFER_LEN, file)) {
		ret = realloc(ret, len + strlen(line) + 1);
		strcpy(ret + len, line);
		len += strlen(line);

		if(len && ret[len - 1] == '\n')
			break;
	}

	if(ret && ret[len - 1] == '\n')
		ret[len - 1] = '\0';

	return ret;
} /* getln() */

int uniq(FILE *in) {
	char *line = NULL, *last = NULL;

	if(!in)
		return -1;

	while((line = getln(in))) {
		if(!last || strcmp(last, line))
			puts(line);

		free(last);
		last = line;
	}

	free(last);
	free(line);
	return 0;
} /* uniq() */

int main(int argc, char *argv[]) {
    // argv[0] is name of program you are running
    // gcc -o my-cat my-cat.c -Wall -Werror
    // ~cs537-1/tests/p1a/test-my-cat.csh
    // ~cs537-1/tests/p1a/test-my-cat.csh -v
    // ./my-cat [file]
	FILE *file = NULL;
	if (argc < 2) {
        printf("my-uniq: [file …]\n");
		exit(1);
	}
    int i, ret = 0;
	for(i = 1; i < argc || file; i++, file = NULL) {
		/* decide where to read the input */
		if(!file) {
			if(!strcmp(argv[i], STDIN_FILE))
				file = stdin;
			else
				file = fopen(argv[i], "r");
		}

		if(!file) {
			perror("uniq");
			ret = 1;
			continue;
		}

		int err = uniq(file);

		if(file != stdin)
			fclose(file);

		if(err < 0) {
			perror("uniq");
			ret = 1;
			continue;
		}
	}
	// else {
    //     for (int i = 1; i < argc; i++) {
    //         FILE *fp = fopen(argv[i], "r");
    //         if (fp == NULL) {
    //             printf("my-uniq: cannot open file\n");
    //             exit(1);
    //         } else {
    //             char* last_line = NULL;
    //             char* line = NULL;
    //             size_t length = 0;
    //             ssize_t read = 0;
    //             int first = 0;
    //             while ((read = getline(&line, &length, fp) != -1)) {
    //                 printf("last_line = %s\nline = %s\n", last_line, line);
    //                 if (first == 1) {
    //                     if (strcmp(last_line, line) || !last_line) {
    //                         printf("!=%s", last_line);
    //                     } else {
    //                         printf("==%s is equal to %s\n", last_line, line);
    //                     }
    //                 }
    //                 last_line = strdup(line);
    //                 first = 1;
    //             }
    //             fclose(fp);
    //         }
    //     }
    // }	
    return ret;
}