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
    else if (argc < 4 || argc > 4) {
		printf("my-sed: find_term replace_term [file …]\n");
        exit(0);
	}
	else {
            char* find_term = argv[1];
            char* replace_term = argv[2];
            for (int i = 3; i < argc; i++) {
			    FILE *fp = fopen(argv[i], "r");
			    if (fp == NULL) {
				    printf("my-sed: cannot open file\n");
				    exit(1);
			    } else {
				    //char buffer[BUFFER_SIZE];
                    printf("looking for %s and replacing it with %s in the file %s", find_term, replace_term, argv[3]);
                    char* line = NULL;
                    size_t length = 0;
                    ssize_t read = 0;
                    while((read = getline(&line, &length, fp))){
                        if (strstr(line, argv[i]) != NULL) {
                            printf("%s", line);
                        }
                    }
                    fclose(fp);
			    }
            }
		}	
	return 0;
}