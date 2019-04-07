#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

  int main(int argc, char * argv[]) {
    if (argc == 1) {
      printf("my-zip: file1 [file2 ...]\n");
	    exit(1);
    }
      char thischar;
      char lastchar;
      int count, total = 0;
    for (int i = 1; i < argc; i++) {
      FILE * fp = fopen(argv[i], "r");
      if (fp == NULL) {
        printf("my-zip: cannot open file\n");
        exit(1);
      }
      while (1) {
          thischar = fgetc(fp);
          if(feof(fp)) break;
          if(total == 0){
            lastchar = thischar;
          }
          total++;
          if(thischar != lastchar){
            fwrite(&count, 4,1,stdout);
            printf("%c", lastchar);
            lastchar = thischar;
            count = 1;
          }
          else{
            count++;
          }
          }
      if(i == argc -1 && thischar == EOF){
        fwrite(&count, 4,1,stdout);
        printf("%c", lastchar);
      }
      fclose(fp);
      }
    return 0;
  }