#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {

	size_t len = 0;
	ssize_t read;
	FILE *fp;
	char *line = NULL;
	char *cptr;
	int result;


	if (argc < 2){
		printf("my-grep: searchterm [file ...]\n");
		exit(1);
	}
	if (argc < 3){
		while ((read = getline(&line,&len,stdin)) != -1) {

		if ((result = strcmp(argv[0], " ")) == 0) {
			return 0;
		}

		cptr = strstr(line, argv[1]);
			if (cptr != NULL) {
				printf("%s", line);
			}
		}
		return 0;
	}	
	for (int i = 2; i < argc; i++){
		
		fp = fopen(argv[i],"r");
		if (fp == NULL) {
			printf("my-grep: cannot open file\n");
			exit(1);
		}

		if ((result = strcmp(argv[0], " ")) == 0) {
			return(0);
		}
		
		while ((read = getline(&line,&len,fp)) != -1) {

			cptr = strstr(line, argv[1]);
			if (cptr != NULL) {
				printf("%s", line);
			}
		}
		
		fclose(fp);
	}
	
	return 0;

}

