#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define BUFFER_SIZE (512)

int main(int argc, char *argv[]) {
	
	if (argc == 0) {
		exit(1);
	}
	else if (argc == 1) {
		exit(0);
	}	
    else if (argc <= 2 || argc > 3) {
		printf("my-sed: find_term replace_term [file …]\n");
        exit(0);
	}
	else {
		for (int i = 1; i < argc; i++) {
            char* find_term = argv[1];
            char* replace_term = argv[2];
            if (find_term == NULL || replace_term == NULL) {

            }
            for (int i = 3; i < argc; i++) {
			    FILE *fp = fopen(argv[i], "r");
			    if (fp == NULL) {
				    printf("my-sed: cannot open file\n");
				    exit(1);
			    } else {
				    char buffer[BUFFER_SIZE];
                    char* line = NULL;
                    size_t length = 0;
                    ssize_t read = 0;
                    while((read = getLine(&line, &length, fp))){
                        if (strstr(line, argv[1] != NULL)) {
                            printf("%s", line);
                        }
                    }
			    }
            }
		}	
	}
	return 0;
}