#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



#define TOKEN_BUFSIZE 100
#define TOKEN_DELIMITER " \t\r\a\n"
char ***split_line(char *line)
{
	int size = TOKEN_BUFSIZE;
	char **argArr = malloc(size * sizeof(char*));
	
	for (int i = 0; i < size; i++){
		argArr[i] = malloc(size * sizeof(char));
		argArr[i] = NULL;
		
	}

	int count = 0;
	
	for (char * token = strtok(line, "&"); token != NULL; token = strtok(NULL, "&")){
		argArr[count] = token;
		count++;
	}
	
	char ***argsArr = malloc(size * sizeof(char**));
	for (int i = 0; i < 100; i++){
		argsArr[i] = malloc(size * sizeof(char*));
	}
	
	for (int i = 0; i < 100; i++){
		for (int j = 0; j < 100; j++){
			argsArr[i][j] = malloc(size * sizeof(char));
			argsArr[i][j] = NULL;
		}
	}

	for (int i = 0; i < count; i++){
		int counter = 0;
		for (char *token = strtok(argArr[i], TOKEN_DELIMITER); token != NULL; token = strtok(NULL, TOKEN_DELIMITER)){
			argsArr[i][counter] = token;
			counter++;
		}	
	}
	return argsArr;



/*        int bufsize = TOKEN_BUFSIZE;
        char **tokenArr = malloc(bufsize * sizeof(char*));

        for (int i = 0; i < 100; i++){
                tokenArr[i] = malloc(bufsize * sizeof(char));
        }

        int position = 0;
	char *token;
	
	token = strtok(line, "&");
	while (token != NULL) {
		tokenArr[position] = token;
		position++;

		token = strtok(NULL, "&");
	}
	tokenArr[position] = NULL;

        char ***tokensArr = malloc(bufsize * sizeof(char**));

        for (int i = 0; i < bufsize; i++){
                tokensArr[i] = malloc(bufsize * sizeof(char*));
        }

        for (int i = 0; i < bufsize; i++){
                for (int j = 0; i < 100; j++){
                        tokensArr[i][j] = malloc(bufsize * sizeof(char));
                }
        }

        for (int i = 0; i < position; i++){
                int position = 0;
		token = strtok(tokenArr[i], TOKEN_DELIMITER);
                while (token != NULL){
                        tokensArr[i][position] = token;
			position ++;

			token = strtok(NULL, TOKEN_DELIMITER);
		}
		tokensArr[i][position] = NULL;
        }

        return tokensArr;
*/
}

char *read_line(int mode)
{
        char *line = NULL;
        size_t bufsize = 0;
        if (mode == 0){
                if (getline(&line, &bufsize, stdin) == -1){
                        exit(0);
                }
        }
        return line;
}

int main(int argc, char **argv){
	while(1){
	printf("testsh> ");
	char *line = read_line(0);
	char ***args = split_line(line);
		
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			printf("%s", args[i][j]);
		}
		printf("\n");
	}	
	}
}
