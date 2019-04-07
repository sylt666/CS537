#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){

	FILE *fp;

	int wordlen = 0;
	char saveChar;
	char compareChar;

	if (argc < 2){
		printf("my-zip: file1 [file2 ...]\n");
		exit(1);
	}

	for (int i = 1; i < argc; i++){

		fp = fopen(argv[i],"r");
		if (fp == NULL) {
			printf("my-cat: cannot open file\n");
			exit(1);
		}

		do {

			compareChar = fgetc(fp);
			if(compareChar == EOF){

				break;
			}

			if(wordlen == 0){

				saveChar = compareChar;
				wordlen++;
			} 
			else{

				if(compareChar == saveChar){
					
					wordlen++;
				} 
				else{
					fwrite(&wordlen, sizeof(int), 1, stdout);
					fwrite(&saveChar, sizeof(char), 1, stdout);
					wordlen = 1;
					saveChar = compareChar;
				}
			}

		} while(compareChar != EOF);

	}

	fwrite(&wordlen, sizeof(int), 1, stdout);
	fwrite(&saveChar, sizeof(char), 1, stdout);

	fclose(fp);

	return 0;

}
