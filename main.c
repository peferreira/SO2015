#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>

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
    double startTime;
    double remainingTime;
    pthread_t *thread;
    int id;
    int running;
    int created;
    int finished;
    sem_t semaphore;
};

double t0[] = {3, 7,8,9,10};
double dt[] = {20,5,2,15,15};
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

void *thread_function(void *context) {

  int i; 
  struct process *proc = context;
  printf("\n");

  while (1) {

    sem_wait(&proc->semaphore);
    /* semáforo usado para bloquear o processo */
    sem_post(&proc->semaphore);

    i++;
    if (i == MAX) {

        i = 0;
    }

  }

  return NULL;
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

int compare_remainingTime(const void *p1, const void *p2)
{
    const struct process *elem1 = p1;    
    const struct process *elem2 = p2;

   if ( elem1->remainingTime < elem2->remainingTime)
      return -1;
   else if (elem1->remainingTime > elem2->remainingTime)
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
        printf("%f\n", array[i].remainingTime);
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
        processArray[i].created = 0;
        processArray[i].finished = 0;
        processArray[i].startTime = 0;
        processArray[i].remainingTime = dt[i];
        sem_init(&(processArray[i].semaphore), 0, 1);
        //processArray[i].name = name[i];
        processArray[i].thread = malloc(sizeof(pthread_t));
    }
/*
    for (i = 0; i < numThreads; i++) {

        printf("Process %d\n", i);
        printf("%f\n", processArray[i].t0);
        printf("%f\n", processArray[i].dt);
        printf("%f\n", processArray[i].deadline);
        printf("%f\n", processArray[i].p);
        printf("%f\n", processArray[i].running);
        //printf("%s\n", processArray[i].name);
    }*/
}

void printProcessInfo(struct process p) {

    printf("t0 : %f\t", p.t0);
    printf("dt: %f\n", p.dt);

}

void printTime(double cpu_time_used) {

    printf("Clock: %f\n", cpu_time_used);
}

int createThread(struct process *p, double cpu_time_used) {

    printf("Criando thread em %f\n", cpu_time_used);
    if (pthread_create(p->thread, NULL, thread_function, p)) {

        printf("Error creating thread.");
        return 1;
    }

    p->startTime = cpu_time_used;
    p->running = 1;
    p->created = 1;

    return 0;
}

void startThread(struct process *p,double cpu_time_used){
    sem_post(&p->semaphore);
    p->running = 1;
    p->startTime = cpu_time_used;
    p->dt = p->remainingTime;
}

int killProcesses(double cpu_time_used) {

    int i, count;
    count = 0;

    for (i = 0; i < numThreads; i++) {


        if (cpu_time_used > (processArray[i].dt + processArray[i].startTime) ) {
                if (processArray[i].running == 1) {
                      pthread_kill(*(processArray[i].thread), SIGCHLD);
                      printf("Thread %d morreu em %f\n", i, cpu_time_used);
                      count++;
                      processArray[i].running = 0;
                      processArray[i].finished = 1;
                }
         }
    }

    return count;
}

void updateRemainingTimes(double cpu_time_used) {

	int i;

	for (i = 0; i < numThreads; i++) {
		if (processArray[i].running == 1) {

            //printf("remaining time anterior %f\n", processArray[i].remainingTime);
            printf("cpu time %f\n", cpu_time_used);
            //printf("startTime %f\n", processArray[i].startTime);

			processArray[i].remainingTime = processArray[i].remainingTime - (cpu_time_used - processArray[i].startTime);
            if (processArray[i].remainingTime < 0) {
                //processArray[i].remainingTime = 0;
                //printf("Clock: %f  Remaining time negativo %f\n", cpu_time_used, processArray[i].remainingTime);sleep(5000);
            }

		}
	}
}

struct process *selectTheLongestRemainingTime() {

	int i;

	for (i = numThreads-1; i >= 0; i--) {
		if (processArray[i].running == 1) {

			return &processArray[i];
		}
	}
}

void substituteProcess(struct process *new_p, struct process *old_p, double cpu_time_used) {

    printf("Vai bloquear processo de t0 %f\n", old_p->t0);
	sem_wait(&old_p->semaphore);
    old_p->running = 0;

	if (new_p->created == 0) {

		createThread(new_p, cpu_time_used);
        printf("Substitute criando processo de t0 %f\n", new_p->t0);

	}
	else {
	
        printf("Vai startar o processo de t0 %f\n", new_p->t0);
        sem_post(&new_p->semaphore); 
	}

        printf("Bloqueou processo de t0 %f\n", old_p->t0);
}

void firstComeFirstServed() {

    printf("\nFIRST COME FIRST SERVED\n");

    clock_t start, end;
    double cpu_time_used;
    int i, k, runningThreads, killedProcesses, delta;
    k = runningThreads = killedProcesses = 0;

    start = clock();

    qsort(processArray, numThreads,sizeof(struct process),compare_t0);
    printArray(processArray);

    while (killedProcesses < numThreads) {

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        if (k < numThreads && cpu_time_used >= processArray[k].t0 && (runningThreads < cores)) {
            
            if (createThread(&processArray[k], cpu_time_used) == 0) {
                printProcessInfo(processArray[k]);
                runningThreads++;
                k++;
            }
            else abort();
        }

        if (runningThreads > 0) {

            delta = killProcesses(cpu_time_used);
            runningThreads -= delta;
            killedProcesses += delta;
        }
    }
}

void shortestJobFirst() {

    printf("\nSHORTEST JOB FIRST\n");

    clock_t start, end;
    double cpu_time_used;
    int i, k, killedProcesses, runningThreads, delta;
    killedProcesses = runningThreads = 0;

    start = clock();

    qsort(processArray, numThreads,sizeof(struct process),compare_dt);
    printArray(processArray);

    while (killedProcesses < numThreads) {

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        k = 0;

        while (k < numThreads) {

            if (cpu_time_used >= processArray[k].t0 && runningThreads < cores && processArray[k].created  == 0) {
                if (createThread(&processArray[k], cpu_time_used) == 0) {

                    printTime(cpu_time_used);
                    printProcessInfo(processArray[k]);
                    runningThreads++;
                }
                else abort();
            }
            k++;
        }

    
        if (runningThreads > 0) {

            delta = killProcesses(cpu_time_used);
            runningThreads -= delta;
            killedProcesses += delta;
        }
    }
}

// remaining time = dt - (cpu_time - startTime)

void shortestRemainingTime() {

    printf("\nSHORTEST REMAINING TIME\n");

    clock_t start, end;
    double cpu_time_used;
    int i, k, killedProcesses, runningThreads, delta;
    killedProcesses = runningThreads = 0;
    struct process *newProcess, *longest;

    start = clock();

    qsort(processArray, numThreads,sizeof(struct process),compare_remainingTime);
    printArray(processArray);

    while (killedProcesses < numThreads) {

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        k = 0;

        while (k < numThreads) { 
            if (cpu_time_used >= processArray[k].t0 && processArray[k].finished == 0 && processArray[k].running == 0) {
                if (runningThreads < cores) { 

                    if (processArray[k].created  == 0) {

                        if (createThread(&processArray[k], cpu_time_used) == 0) {

                            printTime(cpu_time_used);
                            printProcessInfo(processArray[k]);
                        }
                        else abort();                   
                    } 
                    else {
                       startThread(&processArray[k], cpu_time_used);
                    }
                    runningThreads++;   
                }
                
              
                else {   //printf("Tentar trocar processo em %f\n", cpu_time_used);

                	newProcess = &processArray[k];
                    //printf("%f\n", cpu_time_used);
                    //printf("antes\n");
            		//printArray(processArray);
                    qsort(processArray, numThreads,sizeof(struct process),compare_remainingTime);
                    //printf("depois\n");
                    printArray(processArray);
            		longest = selectTheLongestRemainingTime();
                    printf("remaining time do longest: %f\n", longest->remainingTime);
                    printf("remain time do novo: %f\n", newProcess->remainingTime);
            		if (newProcess->remainingTime < longest->remainingTime) {
                        printf("Novo processo tem que entrar\n");
            			substituteProcess(newProcess, longest, cpu_time_used);
            		}
        		//printProcessInfo(*newProcess);
        	   }
            } 
        k++;
        }
        updateRemainingTimes(cpu_time_used);

        if (runningThreads > 0) {

            delta = killProcesses(cpu_time_used);
            runningThreads -= delta;
            killedProcesses += delta;
        }
    }
}

int main() {

    int i;
    numThreads = 5;
    cores = 1;

    mallocProcessArray();
    
    shortestRemainingTime();

    //joinThreads();


  exit(0);
}
