#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(){
  int i;
  const char execute[] = "/bin/ls";
  size_t bytes_read;
  size_t buffer_size = 100;
  char *buffer;
  char *token = NULL;
  char *argv[]={"ls","",NULL};
  
  buffer = (char *)malloc(buffer_size * sizeof(char));
  while(1){
    getline(&buffer, &buffer_size, stdin);
    token = strtok(buffer," ");
    argv[1] = strtok(NULL,"\n");
    execv(token,argv);
  }
  
  
  printf("\n");
  
  //execv(execute,argv);
  return 0;
}
