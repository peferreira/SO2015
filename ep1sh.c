#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

char path[1024];   
char pathBuff[1024];

void setUpPath() {

    int i;
    for (i = 0; i < 1024; i++)
      path[i] = '\0';
    strcat(path, "[");
    strcat(path, pathBuff);
    strcat(path, "] ");
}

int main(){


    char *buf;
    char path2[1024];
    char *token = NULL;
    char *argv[]={"ls","",NULL};
    int *status;

    getcwd(pathBuff, 1024);
    setUpPath();
    buf = readline(path);

    while(buf != NULL) {
      

      if (strcmp(buf,"quit")==0)
        break;

      if (buf[0]!=0)
        add_history(buf);
      token = strtok(buf," ");
      argv[1] = strtok(NULL,"\n");

      if (fork() != 0) { 

        waitpid(-1,&status,0);
      }
      else {

        if (strcmp(token,"cd")==0) {
          chdir(argv[1]);
          getcwd(path2, 1024);
          strcpy(pathBuff,path2);
          setUpPath();
        }
        else if (strcmp(token, "pwd") == 0) {
          getcwd(path2, 1024);
          printf("%s\n", path2);
          strcpy(pathBuff,path2);
          setUpPath();
        }
        else execve(token,argv, 0);
      }
       buf = readline(path);
    }
    free(buf);

    return 0;
}