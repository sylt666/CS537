#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

char *line, *filename;
char ***args;
int position, redir, file_index, error;
FILE *batchf;

// function that replaces the orig str to the rep str
char *replace_str(char *str, char *orig, char *rep)
{
	static char buffer[4096];
	char *p;

	if(!(p = strstr(str, orig)))
    		return str;

  	strncpy(buffer, str, p-str);
  	buffer[p-str] = '\0';

  	sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  	return buffer;
}

// function that gets the number of arguments in a command
int get_argument(char **args){
	int count = 0;
	while (args[count] != NULL){
		count++;
	}	
	return count;
}

// function that gets the number of commands
int get_command(char *** args){
	int count = 0;
	while (args[count][0] != NULL){
		count++;	
	}
	return count;
}

// function that prints error
void print_error()
{
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
}

// function that forks and executes
int launch(char **args, char** paths, int length, int mode, int index)
{
	char *last, *second, *first;
	pid_t pid;
	int status, error = 0;	
	pid = fork();
	if (pid == 0){
		int fd_out;

		// if there is redirection
		if (mode == 1){
			if (args[index + 1] != NULL){
				print_error();
			}
			if ((index-1) == 0){
				print_error();
			}
			close(1);
			fd_out = open(args[index],O_CREAT|O_WRONLY|O_TRUNC, 0666);
			args[index - 1] = NULL;
			if (fd_out < 0){
				print_error();
				exit(1);
			}
	
		}
		if (length != 0){
			for (int i = 0; i < length; i++){
				// strcat the paths
                        	first = strdup(paths[i]);
                       		second = strcat(first, "/");
                       		last = strcat(second,args[0]);
	
				// check for access, if ok, then execv
                        	if (access(last, X_OK) == 0){
                                	if(execv(last, args) == -1){
                                        	error = 1;
                                	}
                                	else{
                                        	error = 0;
                                	}
                        	}
                       		else{
                                	error = 1;
                        	}
               		}
			if (error == 1){
				print_error();
			}
		}
		else{
			print_error();
		}
	}
	else if (pid < 0){
		print_error();	
	}
	else{
		do{
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	

	return 1;
}

// function that checks for redirection
int check_redir(char ***args)
{
	for (int i = 0; i < get_command(args); i++){
                position = get_argument(args[i]);

                int numRe = 0;
		redir = 0;
                for (int j = 0; j < position; j++){
                        if (strcmp(args[i][j],">") == 0){
                                redir = 1;
                                numRe++;
                               	file_index = j + 1;
				break;
                        }
                }
                if (numRe > 1){
                        redir = 0;
			printf("7\n");
                        print_error();
                }
	}

	return redir;
}

// function that parses the line
#define TOKEN_BUFSIZE 1024
#define TOKEN_DELIMITER " \t\r\a\n"
char ***split_line(char *line)
{
	int counter = 0; // tokenCount = 0;
	int size = TOKEN_BUFSIZE;
        char **argArr = malloc(size * sizeof(char*));
        for (int i = 0; i < size; i++){
                argArr[i] = malloc(size * sizeof(char));
                argArr[i] = NULL;
        }

        int count = 0;
	// tokenize by &
        for (char * token = strtok(line, "&"); token != NULL; token = strtok(NULL, "&")){
                argArr[count] = token;
                count++;
        }

        char ***argsArr = malloc(size * sizeof(char**));
        for (int i = 0; i < size; i++){
                argsArr[i] = malloc(size * sizeof(char*));
        }

        for (int i = 0; i < size; i++){
                for (int j = 0; j < size; j++){
                        argsArr[i][j] = malloc(size * sizeof(char));
                        argsArr[i][j] = NULL;
                }
        }
	//add whitespace before and after ">"
	for (int i = 0; i < count; i++){
		argArr[i] = replace_str(argArr[i], ">", " > ");
	}


	//tokenize by whitespace
        for (int i = 0; i < count; i++){
                counter = 0;
                for (char *token = strtok(argArr[i], TOKEN_DELIMITER); token != NULL; token = strtok(NULL, TOKEN_DELIMITER)){
                        argsArr[i][counter] = token;
                        counter++;
                }
        }
        return argsArr;
}

// function that reads the line from either stdin or a file
char *read_line(int mode)
{
	char *line = NULL;
	size_t bufsize = 0;

	// interactive mode
	if (mode == 0){
		if (getline(&line, &bufsize, stdin) == -1){
			exit(0);
		}
	}

	// batch mode
	else{
		if (getline(&line, &bufsize, batchf) == -1){
			exit(0);
		}
	}
	return line;
}

#define BUFSIZE 1024
int main(int argc, char **argv)
{
	int has_redir = 0;
	int bufsize = BUFSIZE, pathslen = 1, batchmode = 0;
	char **paths = malloc(bufsize * sizeof(char*));
	paths[0] = "/bin";

	if (argc == 2){
		// run batch mode
		batchf = fopen(argv[1],"r");
		batchmode = 1;
		if (batchf == NULL){
			print_error();
			exit(1);
		}
	}
	else if (argc > 2){
		print_error();
		exit(1);
	}

	while(1)
	{
		// if run on interactive mode
		if (batchmode == 0){
			printf("wish> ");
			fflush(stdout);
		}

		line = read_line(batchmode);
		args = split_line(line);
		has_redir = check_redir(args);

		for (int i = 0; i < get_command(args); i++){
			position = get_argument(args[i]);

		// if there are nopath arguments passed
			if (args[i][0] == NULL){
				continue;		
			}
		
		// builtin function exit
			if (strcmp(args[i][0], "exit") == 0){
				if (args[i][1] == NULL){
					exit(0);
				
				}
				else{
					print_error();
				}
			}

		// builtin function cd
			else if (strcmp(args[i][0], "cd") == 0){
				if (position > 2 || position == 1){
					print_error();
					continue;
				}
				else{
					if (chdir(args[i][1]) == -1){
						print_error();
					}
				}
			}
		
		// builtin function path
			else if (strcmp(args[i][0], "path") == 0){
				free(paths);
				paths = NULL;
				pathslen = 0;
				paths = malloc(bufsize * sizeof(char*));
				for (int j = 1; j < position; j++){
					paths[j-1] = args[i][j];
					pathslen++;
				}
			}

			else{ 			
				launch(args[i], paths, pathslen, has_redir, file_index);
			

			}

		}
	}
	
}
