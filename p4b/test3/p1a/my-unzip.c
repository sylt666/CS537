#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

  int main(int argc, char * argv[]) {
    if (argc == 1) {
      printf("my-unzip: file1 [file2 ...]\n");
	    exit(1);
    }
    for (int i = 1; i < argc; i++) {
      FILE * fp = fopen(argv[i], "r");
      if (fp == NULL) {
        printf("my-unzip: cannot open file\n");
        exit(1);
      }
      int count;
      char c;      
/*      while(1)){
        fread(&count, 4, 1, fp);
        printf("%d", count);
        thischar = fgetc(fp);
        printf("%c", thischar);
      }*/
      
      while(1){
        //fseek(fp, 1, SEEK_CUR);
        c = fgetc(fp);
        if(feof(fp)){
          break;
        }
        fseek(fp, -1, SEEK_CUR);
        fread(&count, 4, 1, fp);
        c = fgetc(fp);
        printf("%c", c);
        for(int i = 0; i < count - 1; i++){
          printf("%c", c);
        }
      }
      fclose(fp);
      }
    return 0;
  }