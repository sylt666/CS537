#include<stdio.h>
#include<stdlib.h>
#include<string.h>

char *replaceWord(const char *str, const char *old, const char *new) {
    char *result; 
    int i, cnt = 0; 
    int newWlen = strlen(new); 
    int oldWlen = strlen(old); 
  
    // Counting the number of times old word 
    // occur in the string 
    for (i = 0; str[i] != '\0'; i++) 
    { 
        if (strstr(&str[i], old) == &str[i]) 
        { 
            cnt++; 
  
            // Jumping to index after the old word. 
            i += oldWlen - 1; 
        } 
    } 
  
    // Making new string of enough length 
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1); 
  
    i = 0; 
    while (*str) 
    { 
        // compare the substring with the result 
        if (strstr(str, old) == str) 
        { 
            strcpy(&result[i], new); 
            i += newWlen; 
            str += oldWlen; 
        } 
        else
            result[i++] = *str++; 
    } 
  
    result[i] = '\0'; 
    return result; 
}

int main(int argc, char *argv[]) {
	
	if (argc == 0) {
		exit(1);
	}
	else if (argc == 1) {
		exit(0);
	}	
    else if (argc < 4) {
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
                    char* find_term = argv[1];
                    char* replace_term = argv[2];

                    char* line = NULL;
                    size_t length = 0;
                    ssize_t read = 0;
                    while ((read = getline(&line, &length, fp) != -1)) {
                        if (strstr(line, argv[1]) != NULL) {
                            char *result = NULL; 

                            //printf("Old string: %s\n", line); 
                            result = replaceWord(line, find_term, replace_term); 
                            //printf("New String: %s\n", result); 
                            free(result);                          
                            //printf("%s", line);
                        }
                    }
                    fclose(fp);
			    }
            }
		}	
	return 0;
}