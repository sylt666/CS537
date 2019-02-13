#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>


#define INPUT_SIZE 514
char error_message[30] = "An error has occurred\n";
char *newenv[50]; 
char *envpath = "/bin";
int noofprocesses = 0;
char *builtin[] = {"exit","cd","path"};
int history_count = 0;
char *history[INPUT_SIZE];


void mypipe(int pipepos,int argc,char **argv){
	int fp[2];
	if(pipe(fp)<0){
    write(STDERR_FILENO, error_message, strlen(error_message));		
    return;
	}
	int lastpos=pipepos-1;
	while(lastpos>0 && strcmp(argv[lastpos],"|")) --lastpos;
	int pid=fork();
	switch(pid){
		case -1:
			write(STDERR_FILENO, error_message, strlen(error_message));
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
					write(STDERR_FILENO, error_message, strlen(error_message));
			}
			break;
		default:
			argv[argc]=NULL;
			dup2(fp[0],STDIN_FILENO);
			close(fp[0]);
			close(fp[1]);
			while(wait(NULL)>0);
			if(execvp(argv[pipepos+1],&argv[pipepos+1])==-1)
				write(STDERR_FILENO, error_message, strlen(error_message));
			break;
	}	
}


char *parse_ulong(char *src, long *to)
{
    char    *end;
    long  val;

    if (!src) {
        errno = EINVAL;
        return NULL;
    }

    end = src;
    errno = 0;
    val = strtoul(src, (char **)(&end), 0);
    if (errno)
      return NULL;
    if (end == src) {
        errno = EINVAL;
        return NULL;
    }

    if (to)
       *to = val;

    return end;
}

void addHistory(char *inputline) {
    history[history_count] = strdup(inputline);
	history_count++;
}

void printHistory(int count) {
    // User wants more history than we have, so just print out all our history
    int j = history_count - count;
    if (j < 0) {
        j = 0;
    }
    //    printf("History count is: %d", count);
    if (count > history_count) {
        for(int i = 0; i < history_count; i++){
            printf("%s", history[i]);
        }
    } else {
        for(int i = 0 + j; i < history_count; i++){
            printf("%s", history[i]);
        }
    }    
}

void parallelrun(char *newargv[],int newargc, char * fileName){
	int p = 0, fd= 0;
	for(p=0;p<noofprocesses;p++){
		char *fullname = malloc(strlen(newenv[p])+strlen(newargv[0])+1); //+1 for the null-terminator
		fullname[0] = '\0';
	    strcpy(fullname, newenv[p]);
	    strcat(fullname, "/");
	    strcat(fullname, newargv[0]);
		if(access(fullname,X_OK)==0){
			int rc = fork();
			if(rc==0){	
			    newargv[newargc] = NULL;    
			    if(fileName!=NULL){
			    	// mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
					fd = open(fileName, O_RDWR|O_CREAT|O_TRUNC, 0600);
					dup2(fd, 1);
					close(fd);
				}
				execv(fullname, newargv);
				dup2(2, 1);
				exit(0);						
			}
			break;	
		}
		else{
			if(p==noofprocesses-1){
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
	}
}

void forRedirection(char *currargs[],int newargc){
	if(newargc==0)
		return;
	int i = 0;
	char *newargv[50];		
	while(i<newargc && strcmp(currargs[i],">")!=0){
		newargv[i] = currargs[i];
		i++;
	}
	newargv[i] = NULL;
	if(i==newargc){
		parallelrun(newargv,newargc, NULL);
		return;
	}

	if(i==newargc-1 || i<newargc-2){
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}

	if(i==0){
		write(STDERR_FILENO, error_message, strlen(error_message));
		return;
	}
	
	parallelrun(newargv,newargc-2, currargs[newargc-1]);
	return;
}


void execfn(char *inputline){
	addHistory(inputline);
	int i = 0;
	int pipe = 0;
	int pipepos = 0;
	char *newargv[256  * sizeof(char)];
	int newargc = 0;

	while(i<strlen(inputline) && inputline[i]!='\n'){
		if(inputline[i]!= ' ' && inputline[i]!='\t' ){
			if(inputline[i] == '|'){
				char *temparg = malloc(256  * sizeof(char));	
				temparg[0] = '|';
				temparg[1] = '\0';
				newargv[newargc] = temparg;
				newargc++;
				i++;
				pipe = 1;
				pipepos = 2;
				continue;
			}

			if(inputline[i]=='&'){
				char *temparg = malloc(256  * sizeof(char));	
				temparg[0] = '&';
				temparg[1] = '\0';
				newargv[newargc] = temparg;
				newargc++;
				i++;
				continue;
			}

			if(inputline[i] == '>'){
				char *temparg = malloc(256  * sizeof(char));	
				temparg[0] = '>';
				temparg[1] = '\0';
				newargv[newargc] = temparg;
				newargc++;
				i++;
				continue;
			}

			int t = 0;	
			char *temparg = malloc(256  * sizeof(char));
			while(i<strlen(inputline) && inputline[i]!=' ' && inputline[i] != '\t' && inputline[i]!='\n' && inputline[i]!='\r'){
				if(inputline[i]!= '&' && inputline[i]!='>' && inputline[i] != '|'){			
				temparg[t] = inputline[i];
				i++;
				t++;
				}
				else break;
			}
			temparg[t] = '\0';
			newargv[newargc] = temparg;
			newargc++;
		}
		else{
			i++;
		}
	}


	
	if(newargc==0) return;
	if(strcmp(newargv[0],builtin[0])==0){ 
		if(newargc==1)
			exit(0);
		else	
			write(STDERR_FILENO, error_message, strlen(error_message));
	}
	else if (strcmp(newargv[0], "history") == 0) {
	    char *token;
		long number = 0;
		char *ptr = " ";
	    char *temp;
		
		token = strstr(inputline, ptr); // Get pointer to the '->': "history ->2"
		
		if (newargc == 1) {
		  number = 0;
		} else {
		  temp = parse_ulong(token, &number); // Get only the first number they entered, so "history 20 10 50" will only get 20
		}
		
		if (newargc < 3 && newargc > 0) {
			if (number == 0) {
				printHistory(history_count); // print entire history
			} else {
				printHistory(number); // print most recent n history
			}
		} else {
		  write(STDERR_FILENO,error_message, strlen(error_message));
		}
	}
	else if(strcmp(newargv[0],builtin[1])==0){
		if(newargc!= 2){
			write(STDERR_FILENO, error_message, strlen(error_message));
			return;
		}
		else{
			if(chdir(newargv[1])!=0){
				write(STDERR_FILENO, error_message, strlen(error_message));	
			} 
		}
	}
	else if(strcmp(newargv[0],builtin[2])==0){ // overwrite env variable
		noofprocesses = newargc - 1;
		int j;
		for(j=0;j<newargc -1;j++){
			newenv[j] = newargv[j+1];
		}
	}
	else{	
		if(noofprocesses==0){
			write(STDERR_FILENO, error_message, strlen(error_message));
			return;
		}
		if (pipe == 1) {
			int isBackGround=strcmp(newargv[newargc-1],"&"),stat;
			int pid=fork();
			if (pid == -1) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			} else if (pid == 0) {
				mypipe(pipepos, newargc, newargv);	// PIPE SHIT HERE
				waitpid(pid,NULL,0);
				exit(1);
			} else {
				if(strcmp(newargv[0],"wait")==0){
				if(newargc!=1){
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
				while(wait(&stat)>0);
			}
			else if(isBackGround)
				waitpid(pid,NULL,0);
			}
		} else {
		int i = 0;
		int numCommand = 0;
		while(i<newargc){
			char *currargs[50];
			int j = 0;
			while(i<newargc && strcmp(newargv[i],"&")!=0){
				currargs[j] = newargv[i];
				i++;
				j++;
			}
			numCommand++;
			currargs[j] = NULL;
			forRedirection(currargs, j);
			i++;
		}
		while(numCommand-- >0)
			wait(NULL);	
		}
	}
}


void batch(char *fileName){
	FILE *fp = fopen(fileName, "r");
	size_t length=0;
	if(fp==NULL){
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	char* line = NULL;
	while (getline(&line, &length, fp)!=-1) {
		execfn(line);
	}
}



void user(){
	char *line = NULL;
	size_t length = 0;
	while(1){
		printf("wish> ");
		fflush(stdout);
		if(getline(&line, &length, stdin)!=-1) {
			execfn(line);
		} else break;
	}
}

int main(int argc, char *argv[]){	
	newenv[0] = envpath;
	noofprocesses = 1;	

	if(argc==1){
		user();
	}
	else if(argc==2){
		batch(argv[1]);
	}
	else{
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
	return 0;
}
