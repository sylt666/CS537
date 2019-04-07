#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 300

int main(int argc, char *argv[]) {


	if (argc < 2){
		exit(0);
	}

	for (int i = 1; i < argc; i++){
		
		FILE *fp = fopen(argv[i],"r");
		if (fp == NULL) {
			printf("my-cat: cannot open file\n");
			exit(1);
		}
		char buffer[BUFFER_SIZE];
		while (fgets(buffer, BUFFER_SIZE, fp) != NULL){
		
			printf("%s", buffer);
		}

		fclose(fp);
	}
	
	return 0;

}

