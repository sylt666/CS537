#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#define INPUT_SIZE 514
#define MAXN 1000

char* homePath, curPath[MAXN];
char *newline="\n";
int history_count = 0;
char *history[INPUT_SIZE];
char error_message[30] = "An error has occurred\n";

char *temp = "test\n";
char *temp2 = "test2\n";

// Helper Methods
void printError() {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
}
// End of helper methods

void addHistory(int argc, char **argv) {
    history_count++;
    char temp[INPUT_SIZE] = "";

    for(int i = 0; i < argc; i++) {
      strcat(temp, argv[i]);
      strcat(temp, " ");
    }
    history[history_count] = strdup(temp);
}

void printHistory(int count) {
    // User wants more history than we have, so just print out all our history
    int j = history_count - count;
    if (j < 0) {
        j = 0;
    }
    if (count > history_count) {
        for(int i = 1; i < history_count; i++){
            printf("%d: %s\n", i, history[i]);
        }
    } else {
        for(int i = 1 + j; i <= history_count; i++){
            printf("%d: %s\n", i, history[i]);
        }
    }    
}

void mypipe(int pipepos,int argc,char **argv){
	printf("pipepos = %d\n", pipepos);
	printf("newargc = %d\n", argc);
	for(int i = 0; i < argc; i++) {
		printf("newargv = %s\n", argv[i]);
	}
	int fp[2];
	if(pipe(fp)<0){
    printError();		
    return;
	}
	int lastpos=pipepos-1;
	while(lastpos>0 && strcmp(argv[lastpos],"|")) --lastpos;
	int pid=fork();
	switch(pid){
		case -1:
			printError();
			break;
		case 0:
			dup2(fp[1],STDOUT_FILENO);
			close(fp[0]);
			close(fp[1]);
			if(lastpos!=0)	
				mypipe(lastpos,pipepos,argv);
			else{
				argv[pipepos]=NULL;
				if(execvp(argv[0],argv)==-1)
					printError();
			}
			break;
		default:
			argv[argc]=NULL;
			dup2(fp[0],STDIN_FILENO);
			close(fp[0]);
			close(fp[1]);
			while(wait(NULL)>0);
			if(execvp(argv[pipepos+1],&argv[pipepos+1])==-1)
				printError();
			break;
	}	
}

int getargs(int *argc,char **argv,char *input,int *redpos,int *pipepos){
	char *in=NULL;
	int i,j;
	int len=strlen(input);
	for(i=0;i<len;++i){
		if(input[i]=='<' || input[i]=='>' || input[i]=='&' || input[i]=='|'){
			for(j=len+2;j>i;--j)
				input[j]=input[j-2];
			input[i+1]=input[i];
			input[i+2]=' ';
			input[i]=' ';
			++i;
			len+=2;
		}
	}
	in=input;
	for(i=0;i<MAXN-1;++i){
		if((argv[i]=strtok(in," \t\n"))==NULL)
			break;
		if((!strcmp(argv[i],">") || !strcmp(argv[i],"<")) && !*redpos)
			*redpos=i;
		if(!strcmp(argv[i],"|"))
			*pipepos=i;
		in=NULL;
	}
	if(i>MAXN-1){
		printError();
		return 0;
	}
	*argc=i;
	return 1;
}

void execute(int argc,char **argv,int redpos,int pipepos){
	int isBackGround=strcmp(argv[argc-1],"&"),stat,i;
	if(!isBackGround)
		argv[--argc]=NULL;
	if(strcmp(argv[0],"cd")==0){
		if(argc>1){
			if(chdir(argv[1])==-1){
				printError();
				exit(1);
			}
			getcwd(curPath,MAXN);
		}
		else{
			chdir(homePath);
			getcwd(curPath,MAXN);
		}
		return;
	} else if (strcmp(argv[0], "history") == 0) {
        if (argc == 1) {
            // User typed "history", so print out all history
            printHistory(history_count);

        } else if (isdigit(*argv[1]) == 1 && argc == 2) {
            printHistory(atoi(argv[1])); // Print previous n lines of history
        } else {
            printError();
        }
        return;
    }
	int pid=fork();
	switch(pid){
		case -1:
			printError();
			exit(1);
		case 0:
			for(i=0;i<argc;++i){
				if(argv[i][0]=='$' && getenv(&argv[i][1]))
					argv[i]=getenv(&argv[i][1]);
			}
			if(pipepos){
				if(redpos){
					if(argc-redpos!=2)
						printError();
					else{
						close(STDOUT_FILENO);
						int fp=open(argv[redpos+1],O_CREAT|O_TRUNC|O_WRONLY,(S_IRWXU^S_IXUSR)|S_IRGRP|S_IROTH);
						if(fp==-1){
							printError();
							exit(0);
						}
                        write(STDERR_FILENO, temp, strlen(temp)); //HELP
						argv[redpos]=NULL;
						mypipe(pipepos,argc,argv);
						exit(0);
					}
				}
				else {
                    write(STDERR_FILENO, temp2, strlen(temp2)); //HELP
					mypipe(pipepos,argc,argv);
                }
				exit(1);
			}
			if(strcmp(argv[0],"wait")==0)
				exit(0);
			else if(strcmp(argv[0],"pwd")==0){
				if(argc>1){
					printError();
					exit(0);
				}
				write(STDOUT_FILENO,curPath,strlen(curPath));
				write(STDOUT_FILENO,newline,strlen(newline));
				exit(0);
			}
			else{
				if(!redpos && execvp(argv[0],argv)==-1){
					printError();
					exit(0);
				}
				else{
					if(argc-redpos!=2){
						printError();
						exit(0);
					}
					close(STDOUT_FILENO);
					int fp=open(argv[redpos+1],O_CREAT|O_TRUNC|O_WRONLY,(S_IRWXU^S_IXUSR)|S_IRGRP|S_IROTH);
					if(fp==-1){
						printError();
						exit(0);
					}
					argv[redpos]=NULL;
					if(execvp(argv[0],argv)==-1){
						printError();
						exit(0);
					}
				}
			}
			break;
		default:
			if(strcmp(argv[0],"wait")==0){
				if(argc!=1){
					printError();
					break;
				}
				while(wait(&stat)>0);
				break;
			}
			else if(isBackGround)
				waitpid(pid,NULL,0);
			break;
	}
}

void startWish(int flag) {
    char *argv[MAXN];
	char input[MAXN];
	while(1){

        // Print prompt only if the user did give a Batch file
		if (flag) {
		    write(STDOUT_FILENO,"wish> ", 6);
        }

        int argc = 0, redpos = 0, pipepos = 0;
		int pos = 0, rc;
		
        // Put user input into input[] array
        while(pos < MAXN - 1 && (rc = read(STDIN_FILENO, &input[pos], 1)) && input[pos] != '\n') { 
            ++pos;
        }
		input[pos] = '\0'; // Add NULL onto end of input array
		
        if (pos == 0 && rc == 0) {
			break;
		} else if (pos > 512) {
            // User entered a bunch of jibberish, just print out "wish> " again
			write(STDOUT_FILENO,input,strlen(input));
			write(STDOUT_FILENO,newline,strlen(newline));
			printError();
			continue;
		}

        // User entered batch file
		if (!flag) {
			write(STDOUT_FILENO,input,strlen(input));
			write(STDOUT_FILENO,newline,strlen(newline));
		}
		if (getargs(&argc, argv, input, &redpos, &pipepos) && argc > 0) {
			if (strcmp(argv[0], "exit") == 0) {
				if (argc != 1) {
                    // User typed "exit [something else]"
					printError();
                } else {
                    break; // Exit Wish gracefully
                }
			} else {                
                addHistory(argc, argv);
                execute(argc, argv, redpos, pipepos); // Excecute users command
            }
		}
	}
}


int main(int argc, char *argv[]) {	
	
    homePath = getenv("HOME");
	getcwd(curPath, MAXN - 1);
	int i;
    int fp;

    if (argc > 1) {
        for(i = 0; i < argc; i++) {
            if ((fp = open(argv[i], O_RDONLY)) == -1) {
                printError();
            } else {
                dup2(fp, STDIN_FILENO);
                startWish(0);
            }   
        }
    } else {
        startWish(1);
    }
	return 0;
}