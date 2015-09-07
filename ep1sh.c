#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

int main(){


    char *buf;
    char *token = NULL;
    char *argv[]={"ls","",NULL};
    int *status;


    while((buf = readline("[home/mac/] "))!=NULL) {

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

        execve(token,argv, 0);
      }
    }
    free(buf);

    return 0;
}