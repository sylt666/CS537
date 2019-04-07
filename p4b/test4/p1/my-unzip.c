#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE (512)

int main(int argc, char *argv[]){

	FILE *fp;
	int wordlen;
	char ch;

	if (argc < 2){
		printf("my-unzip: file1 [file2 ...]\n");
		exit(1);
	}

	for (int i = 1; i < argc; i++){

		fp = fopen(argv[i],"r");
		if (fp == NULL) {
			printf("my-cat: cannot open file\n");
			exit(1);
		}
		
		while (fread(&wordlen, sizeof(int), 1, fp) == 1){
			ch = fgetc(fp);

			for (int i = 0; i < wordlen; i++){
				printf("%c", ch);
			}																																							
		}
		
		
		
	}

	fclose(fp);

	return 0;

}																												
