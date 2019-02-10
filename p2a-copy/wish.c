#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#define delim " \t\r\n\a"

int ifRedirect = 1;
int ifPallel = 1;
char *file = NULL;
int num = 1;
char *PATH[100];
char *args[100];

void error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}
int getTokens(char *args[], char *line, char *sep);
int redirect(char *ret, char *line, char *args[]);
int execute(char *args[], int ifRedirect, char* dir);
int parallel(char *pal, char *line);

int getDic(char *path[], char *args[], char *dir){
    if(args == NULL) return 1;
    if(args[0] == NULL) return 1;
    int isfound = 0;
    if (path[0] == NULL) {
        error();
        return 1;
    }
    for(int i = 0; i < num; i++){
        char temp[100];
        strcpy(temp, path[i]);
        int length = strlen(temp);
        temp[length] = '/';
        temp[length + 1] = '\0';
        strcat(temp, args[0]);
        if(access(temp, X_OK) == 0){
            strcpy(dir, temp);
            isfound = 1;
            break;
        }
    }
    if(isfound == 0){
        error();
        return 1;
    }
    if(path[0] == NULL){
        error();
        return 1;
    }
    if(dir == NULL){
        error();
        return 1;
    }
    return 0;
}

int builtIn(char *path[], char *args[]){
    if (strcmp(args[0], "exit") == 0){
        if (args[1] != NULL) error();
        else exit(0);
        return -1;
    }
    if (strcmp(args[0], "cd") == 0){
        if ((!args[1]) || args[2]){
            error();
        }
        else 
            if (chdir(args[1]) == -1) error();
        return -1;
    }
    if (strcmp(args[0], "path") == 0){
        if(args[1] == NULL){
            path[0] = NULL;
        }
        int i = 0;
        for (i = 0; args[i + 1] != NULL; i++){
            path[i] = args[i + 1];
        }
        num = i + 1;
        return -1;
    }

    return 0;
}
int readCommand(char *args[], FILE *fp){
    char *line = NULL;
    size_t size = 0;
    fflush(fp);
    if (getline(&line, &size, fp) == -1){
        //error(); //error
        return 1;
    }
    fflush(stdin);
    if ((strcmp(line, "\n") == 0) || (strcmp(line, "") == 0))
        return -1;

    line[strlen(line) - 1] = '\0'; //clean the '\n'
    if (line[0] == EOF) return 1;

    char *pal = strchr(line, '&');
    if (pal){
        parallel(pal, line);
        return -1;
    }

    char *ret = strchr(line, '>');
    if (ret){
        redirect(ret,line,args);
            //return -1;
        return -1;
    }
    getTokens(args, line, delim);
    if (args[0] == NULL)
        return -1;
    return 0;
}

int getTokens(char *args[], char *line, char *sep){
    int index = 0;
    char *save;
    args[index] = strtok_r(line, sep, &save);
    while(args[index] != NULL){
        index++;
        args[index] = strtok_r(NULL, sep, &save);
    }
    return 0;
}

int redirect(char *ret, char *line, char *args[]){
    ret[0] = '\0';
    ret = ret + 1;
    char *retArguments[10];
    getTokens(args, line, delim);
    getTokens(retArguments, ret, delim);
    file = retArguments[0];
    if(file == NULL){
        error();
        return 1;
    }
    if(args[0] == NULL){
        error();
        return 1;
    }
    if (retArguments[1]) {
        error();
        return 1;
    }
    char dir[100];
    getDic(PATH, args, dir);
    execute(args, 0, dir);
    //ifRedirect = 0;
    return 0;
}

int parallel(char *ret, char *line){
    char *com[100];
    getTokens(com, line, "&");
    char *temp;
    //char *args[100];
    int i = 0;
    char dir[100];
    while(1){
        if (!com[i])
            break;   
        if((temp = strchr(com[i], '>'))){
            redirect(temp,com[i], args);
            i++;
            continue;
        }
        getTokens(args, com[i], delim);
        if (args[0] == NULL)
            break;
        getDic(PATH, args, dir);
        execute(args, 1, dir);
        i++;
    }
    return 0;
}

int execute(char *args[], int ifRedirect, char* dir){
    int childpid;
    int childStatus;
    childpid = fork();
    if(childpid == 0) {
        if (ifRedirect == 0){
            int fd_out = open(file, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            if (fd_out > 0){
                dup2(fd_out, STDOUT_FILENO);
                fflush(stdout);
            }
        }
        if (args[0] == NULL)
            return -1;

        execv(dir,args);
    }else{
        waitpid (childpid, &childStatus, 0);
    }

    ifRedirect = 1;
    return 0;
}

int main(int argc, char *argv[]){
    char dir[100];
    PATH[0] = "/bin";
    if (argc == 2){
        FILE *fp;
        if (!(fp = fopen(argv[1], "r"))){
            error();
            exit(1);
        }
        while(1){
            int read = readCommand(args, fp);
            fflush(fp);
            if (read == -1)
                continue;
            if (read == 1)
                break;
            if(!args[0])
                break;
            //fflush(fp);
            if (builtIn(PATH,args) == -1)
                continue;
            if (getDic(PATH, args, dir) == 1)
                continue;
            //execute process
            if(execute(args, ifRedirect, dir) == -1)
                continue;
        }
        return 0;
    }
    if (argc < 1 || argc > 2){
        error();
        exit(1);
    }
    while(1){
        printf("wish> ");
        fflush(stdout);
        int status = readCommand(args, stdin);
        fflush(stdin);
        if (status == -1) continue;
        if (status == 1) break;

        if (builtIn(PATH,args) == -1)
            continue;
        if (getDic(PATH, args, dir) == 1)
            continue;

        //execute process
        if(execute(args, ifRedirect, dir) == -1)
            continue;
    }
}