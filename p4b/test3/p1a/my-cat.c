#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE (512)
/* */

int main(int argc, char *argv[]){
	//argv[0] is name of the program
	for(int i = 1; i < argc; i++){
	FILE *fp = fopen(argv[i],"r");
	if (fp == NULL) {
		printf("my-cat: cannot open file\n");
		exit(1);
	}
	char buffer[BUFFER_SIZE];
	while(fgets(buffer, BUFFER_SIZE, fp) != NULL)
		printf("%s", buffer);
	fclose(fp);
	}
	return 0;
}
