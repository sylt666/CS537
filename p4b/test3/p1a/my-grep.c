#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

  int main(int argc, char * argv[]) {

    if (argc == 1) {
      printf("my-grep: searchterm [file ...]\n");
	  exit(1);
    }
	if(argc == 2){
	  char * line = NULL;
      size_t len = 0;
      size_t read;
      char * string = argv[1];
      while ((read = getline( & line, & len, stdin)) != -1) {
        if (strstr(line, string) != NULL) {
          printf("%s", line);
        }
      }
	}
	else{
    for (int i = 2; i < argc; i++) {
      FILE * fp = fopen(argv[i], "r");
      if (fp == NULL) {
        printf("my-grep: cannot open file\n");
        exit(1);
      }
      char * line = NULL;
      size_t len = 0;
      size_t read;
      char * string = argv[1];
      while ((read = getline( & line, & len, fp)) != -1) {
        if (strstr(line, string) != NULL) {
          printf("%s", line);
        }
      }
      fclose(fp);
    }
	}
    return 0;
  }