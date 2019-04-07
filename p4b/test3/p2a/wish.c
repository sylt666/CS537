#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


char error_message[30] = "An error has occurred\n";
const int MODE_SPACE = 1;
const int MODE_REDIR = 2;
const int MODE_PARA = 3;



  char **parse(char *cmd, int mode, int * endpos)
  {	
  	int size = 128;
  	char **tokens = malloc(size * sizeof(char*));
  	char *token;
  	char *saveptr;
  	//endpos = 0;
  	
	
 	if(mode == MODE_SPACE){
  		token = strtok_r(cmd, " \n\t\r\a", &saveptr);
	}
	else if(mode == MODE_REDIR){
		token = strtok_r(cmd, "> \n\t\r\a", &saveptr);
	}
	else if(mode == MODE_PARA){
		token = strtok_r(cmd, "&", &saveptr);
	}
  	int pos = 0;
  	while(token != NULL)
  	{
		tokens[pos] = token;

		pos++;
  		if(pos >= size)
  		{
  			size += size;
  			tokens = realloc(tokens, size* sizeof(char*));
  		}
	 	if(mode == MODE_SPACE){
	  		token = strtok_r(NULL, " \n\t\r\a", &saveptr);
		}
		else if(mode == MODE_REDIR){
			token = strtok_r(NULL, "> \n\t\r\a", &saveptr);
		}
		else if(mode == MODE_PARA){
			token = strtok_r(NULL, "&", &saveptr);
		}
  	}
  
  	tokens[pos] = NULL;
	*endpos = pos;
	//free(start);
  	return tokens;
  }


  int main(int argc, char * argv[]) {
	


	int batchmode = 0;
	char *filename;
	FILE *fp;
	if(argc == 1) {
		batchmode = 0;
	}
	else if(argc == 2) {
		filename = argv[1];
		fp = fopen(filename, "r");
		if (fp == NULL) {
			//printf("cannot open file\n");
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);
		}
		batchmode = 1;
	}
	else{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	int stdin_fd = dup(STDIN_FILENO);
	int stdout_fd = dup(STDOUT_FILENO);
	char ** paths = malloc(1000 * sizeof(char*));
	int default_path = 1;
	int run = 1;


	while(run){
		int paraflag = 0;

		//get command

		char * line = NULL;
		size_t len = 0;

		char** commands;					
		if(batchmode == 1){

			if(getline( & line, & len, fp) == -1){
				exit(0);
			}
		}
		else{
			printf("wish> ");
			fflush(stdout);
			if(getline( & line, & len, stdin) == -1) exit(0);
		}

		
		for(int i = 0; i < strlen(line); i++){
			if(line[i] == '&'){
				paraflag = 1;
			}
		}
		//split by '&', get multiple commands		
		int line_endpos = 0;
		if(paraflag){}
		commands = parse(line,MODE_PARA, &line_endpos);
		//}
		for(int i = 0; i < line_endpos; i++){
			int errorflag = 0;
			int redirflag = 0;
			char * command = commands[i];
			int redircount = 0;
			
			for(int j = 0; j < strlen(command); j++){
				if(command[j] == '>'){
					redirflag = 1;
					redircount++;
				}
			}
			if(redirflag){
				if(redircount > 1) errorflag = 1;
				else{
					char *temp_command = (char*)malloc(1+strlen(command));
					strcpy(temp_command, command);
					char *after_redir = strchr(temp_command, '>');
					int temp_endpos = 0;
					parse(after_redir, MODE_REDIR, &temp_endpos);
					if(temp_endpos != 1){
						errorflag = 1;
					}
				}
			}

			int command_endpos = 0;
			char** args;
			args = parse(command, MODE_REDIR, &command_endpos);
			
			
			
			//if no arg, skip
			if(args[0] == NULL) continue;


			//build-in commend
			if(strcmp(args[0], "path") == 0){
				default_path = 0;
				//char *path;
				int i = 1;
				while(1){
					paths[i-1] = args[i];
					if(args[i] == NULL) break;				
					i++;			
				}
			}
			else if(strcmp(args[0], "exit") == 0){
				if(args[1] != NULL){
					 write(STDERR_FILENO, error_message, strlen(error_message)); 
				}
				else exit(0);
			}
			else if(strcmp(args[0], "cd") == 0){
				if(args[1] != NULL && args[2] == NULL){
					if(chdir(args[1]) != 0) {
						printf("cd error\n");
					}
				}
				else write(STDERR_FILENO, error_message, strlen(error_message));
			}


			//basic function
			else{
				char * path = (char *) malloc(1 + 5 * sizeof(char));
				strcpy(path, "/bin/");

				if(default_path){
					char * start = (char *) malloc(1 + strlen(args[0])+ strlen(path) );
					strcpy(start, path);
					strcat(start, args[0]);
					if(access(start, X_OK) != 0){
						free(start);	
						errorflag = 1;			
					}
				}	
				else{
					if(paths[0] == NULL){
						path = NULL;
					}
					else{
						int i = 0;
						int accessed = 0;
						while(1)
						{
							if(paths[i] == NULL) break;
						
							path = (char *) realloc (path, 1 + strlen(paths[i]));
							strcpy(path, paths[i]);

							if(path[strlen(path) -1] != '/'){
								strcat(path, "/");			
							}

							char * start = (char *) malloc(1 + strlen(args[0])+ strlen(path) );
							strcpy(start, path);
							strcat(start, args[0]);
							if(access(start, X_OK) == 0){
								free(start);	
								accessed = 1;
								break;				
							}
						i++;
						}
						if(accessed == 0) errorflag = 1;
					}
				}

				char * start;
				if(path != NULL){
					start = (char *) malloc(1 + strlen(args[0])+ strlen(path) );			
					strcpy(start, path);
					strcat(start, args[0]);
					//args[0] = start;
				}
				else{
					errorflag = 1;			
				}

				//redirect
				//int j = 0;
				//while(1){
				//	if(args[j] == NULL) break;
				//	if(strcmp(args[j], ">") == 0){
				//		if(j == 0 || args[j+1] == NULL || args[j+2] != NULL) {
				//			errorflag = 1;
				//			break;
				//		}
				//		close(1);
				//		char *redirfile = strdup(args[j+1]);
				//		int fd_out = open(redirfile, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				//		dup2(fd_out, 1);
				//		args[j] = NULL;
				//		break;
				//	}
				//	j++;
				//	
				//}

				//redirection rewrite

				if(redirflag == 1){
					char *redirfile = args[command_endpos - 1];
					//command_endpos = command_endpos - 1;
					int fd_out = open(redirfile, O_CREAT | O_RDWR, S_IRUSR);
					dup2(fd_out, 1);
					args[command_endpos - 1] = NULL;
					command_endpos = command_endpos - 1;
				}

				//display error		
				if(errorflag) {
					write(STDERR_FILENO, error_message, strlen(error_message));	
					continue;
				}
			

				//excute
				int pid = fork();
				if(pid == 0){
					//child process

					execv(start, args);
				
				}
				else{
					pid = (int) wait(NULL);
				}
			}
		dup2(stdin_fd, 0);			
		dup2(stdout_fd, 1);
		}
	}
	//free(paths);
	return 0;
  }
