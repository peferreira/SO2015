#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define MAX 1000000
#define CORES 1

int numThreads;
int cores;
long timer;

struct process {

    double t0;
    char *name;
    double dt;
    double deadline;
    int p;
    long elapsedTime;
    pthread_t *thread;
    int id;
    int running;
};

double t0[] = {1,2,3,4,5};
double dt[] = {0.1,0.1,0.1,0.1,0.1};
double deadline[] = {44,55,11,22,33};
int p[] = {11,4,3,2,1};
char name[] = {'a','b','c','d','e'};

struct process *processArray;

void decrementCores() {

    cores--;
}

void incrementCores() {

    cores++;
}

void *thread_function(void *arg) {

  int i;
     printf("Hi\n", i);


  while (1) {

    i++;

    if (i == MAX) {

        //printf("%d\n", i);
        i = 0;
    }

  }

  return NULL;
}

void createThread(pthread_t *threadName) {

    printf("Criando thread\n");
    if (pthread_create(threadName, NULL, thread_function, NULL)) {

        printf("error creating thread.");
        abort();
    }
    printf("thread criado com sucesso\n");

}

void joinThreads() {

    int i;

    for (i = 0; i < numThreads; i++) {
        if (pthread_join(*processArray[i].thread, NULL)) {

            printf("error joining thread.");
            abort();
        }
    }
}

int compare_ints (const void *a, const void *b)
{
  const int *da = (const int *) a;
  const int *db = (const int *) b;

  return (*da > *db) - (*da < *db);
}

int compare_t0(const void *p1, const void *p2)
{
    const struct process *elem1 = p1;    
    const struct process *elem2 = p2;

   if ( elem1->t0 < elem2->t0)
      return -1;
   else if (elem1->t0 > elem2->t0)
      return 1;
   else
      return 0;
}

int compare_dt(const void *p1, const void *p2)
{
    const struct process *elem1 = p1;    
    const struct process *elem2 = p2;

   if ( elem1->dt < elem2->dt)
      return -1;
   else if (elem1->dt > elem2->dt)
      return 1;
   else
      return 0;
}

int compare_deadline(const void *p1, const void *p2)
{
    const struct process *elem1 = p1;    
    const struct process *elem2 = p2;

   if ( elem1->deadline < elem2->deadline)
      return -1;
   else if (elem1->deadline > elem2->deadline)
      return 1;
   else
      return 0;
}

int compare_p(const void *p1, const void *p2)
{
    const struct process *elem1 = p1;    
    const struct process *elem2 = p2;

   if ( elem1->p < elem2->p)
      return -1;
   else if (elem1->p > elem2->p)
      return 1;
   else
      return 0;
}

void printArray(struct process *array) {

    int i;
    printf("Array:\n");
    for (i = 0; i < numThreads; i++) 
        printf("%f\n", array[i].t0);
}

void mallocProcessArray() {

    int i;

    processArray = malloc(numThreads * sizeof(struct process)); 
    
    for (i = 0; i < numThreads; i++) {

        processArray[i].t0 = t0[i];
        processArray[i].dt = dt[i];
        processArray[i].deadline = deadline[i];
        processArray[i].p = p[i];
        processArray[i].running = 0;
        //processArray[i].name = name[i];
        processArray[i].thread = malloc(sizeof(pthread_t));
    }

    for (i = 0; i < numThreads; i++) {

        printf("Process %d\n", i);
        printf("%f\n", processArray[i].t0);
        printf("%f\n", processArray[i].dt);
        printf("%f\n", processArray[i].deadline);
        printf("%f\n", processArray[i].p);
        printf("%f\n", processArray[i].running);
        //printf("%s\n", processArray[i].name);
    }
}

void firstComeFirstServed() {

    printf("\nFIRST COME FIRST SERVED\n");

    clock_t start, end;
    double cpu_time_used;
    int i;
    int k = 0;
    int runningThreads = 0;

    start = clock();

    qsort(processArray,5,sizeof(struct process),compare_t0);
    printArray(processArray);

    while (k <= numThreads) {

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        if (cpu_time_used >= processArray[k].t0 && runningThreads < cores) {
            createThread(processArray[k].thread);
            printf("%d: %f\n", k, processArray[k].t0);
            processArray[k].running = 1;
            runningThreads++;
            k++;
        }

        if (runningThreads > 0)
        {
            for (i = 0; i < k; i++) {


                if (cpu_time_used > (processArray[i].dt + processArray[i].t0) ) {
                    if (processArray[i].running == 1) {
                        printf("vou tentar matar o processo\n");
                        pthread_kill(*(processArray[i].thread), SIGCHLD);
                        printf("Thread %d morreu\n", i);
                        runningThreads--;
                        processArray[i].running = 0;
                    }
                }
            }
            
        }
    }
}

int main() {

    int i;
    numThreads = 5;
    cores = 1;

    mallocProcessArray();
    
    firstComeFirstServed();

    //joinThreads();


  exit(0);
}
