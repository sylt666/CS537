#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
    // argv[0] is name of program you are running
    // gcc -o my-cat my-cat.c -Wall -Werror
    // ~cs537-1/tests/p1a/test-my-cat.csh
    // ~cs537-1/tests/p1a/test-my-cat.csh -v
    // ./my-cat [file]
	
	if (argc == 0) {
		exit(1);
	}
	else if (argc == 1) {
		exit(0);
	}	
    else if (argc < 4 || argc > 4) {
		printf("my-sed: find_term replace_term [file …]\n");
        exit(0);
	}
	else {
            for (int i = 3; i < argc; i++) {
			    FILE *fp = fopen(argv[i], "r");
			    if (fp == NULL) {
				    printf("my-sed: cannot open file\n");
				    exit(1);
			    } else {
				    //char buffer[BUFFER_SIZE];
                    char* find_term = argv[1];
                    char* replace_term = argv[2];
                    printf("looking for %s, replaceing with %s, looking in file: %s\n",find_term, replace_term, argv[3]);
                    char* line = NULL;
                    size_t length = 0;
                    ssize_t read = 0;
                    while ((read = getline(&line, &length, fp) != -1)) {
                        if (strstr(line, argv[1]) != NULL) {
                            printf("%s", line);
                        }
                    }
                    fclose(fp);
			    }
            }
		}	
	return 0;
}