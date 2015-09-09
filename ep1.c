#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>

#define MAX 1000000
#define NPROCSMAX 100
#define CORES 1
#define NAMESIZEMAX 150

int numThreads;
int nprocs;
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

/*double t0[] = {3, 7,8,9,10};
double dt[] = {5,5,2,15,15};
double deadline[] = {44,55,11,22,33};
int p[] = {11,4,3,2,1};
char name[] = {'a','b','c','d','e'};*/
double t0[NPROCSMAX];
double dt[NPROCSMAX];
double deadline[NPROCSMAX];
int p[NPROCSMAX];
char **name;

struct process *processArray;

void printIncomingProcess(){
    int i;
    for(i = 0; i < nprocs; i++){
        printf("%f %s %f %f %d\n", t0[nprocs], name[nprocs], dt[nprocs], deadline[nprocs], p[nprocs]);    
    }
}

void readInput(){
        int i;
	    char    *buffer;
        char *t0_in, *dt_in, *deadline_in, *p_in, *name_in;
        size_t  n = 1024;
        nprocs = 0;
        buffer = malloc(n);
        name = malloc(NPROCSMAX*sizeof(char *));
       
        while ((getline(&buffer, &n, stdin) != -1)) {
                t0_in = strtok(buffer," ");
                t0[nprocs] = atof(t0_in);
                name_in = strtok(NULL," ");
                name[nprocs] = name_in;
                dt_in = strtok(NULL," ");
                dt[nprocs] = atof(dt_in);
                deadline_in = strtok(NULL," ");
                deadline[nprocs] = atof(deadline_in);
                p_in = strtok(NULL,"\r\n");
                p[nprocs] = atoi(p_in);
                printf("%s %s %s %s %s\n", t0_in, name_in, dt_in, deadline_in, p_in);
                printf("%f %s %f %f %d\n", t0[nprocs], name[nprocs], dt[nprocs], deadline[nprocs], p[nprocs]);
                nprocs++;
        }
        

        for(i = 0; i < nprocs; i++){
            printf("%f %s %f %f %d\n", t0[nprocs], name[nprocs], dt[nprocs], deadline[nprocs], p[nprocs]);    
        }
}

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

  while (!proc->finished)  {

    sem_wait(&proc->semaphore);
    /* semáforo usado para bloquear o processo */
    sem_post(&proc->semaphore);

    i++;
    if (i == MAX) {

        printf("Rodando %f\n", proc->t0);
        i = 0;
    }

  }
  printf("thread t0: %f vai morrer\n", proc->t0);

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
        sem_init(&(processArray[i].semaphore), 0, 0);
        /*processArray[i].name = name[i];*/
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
    printf("remaining time: %f\n", p.remainingTime);
    printf("startTime: %f\n", p.startTime);
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

    p->running = 0;
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
                      /*sem_wait(&processArray[i].semaphore);
                      if(pthread_kill(*(processArray[i].thread), SIGCHLD)== 0){*/
                        printf("Thread de t0 %f morreu em %f\n", processArray[i].t0, cpu_time_used);
                        count++;
                        processArray[i].running = 0;
                        processArray[i].finished = 1;
                
                }
         }
    }

    return count;
}

void updateElapsedTimes(double elapsedTime) {

    int i;

    for (i = 0; i < numThreads; i++) {
        if (processArray[i].running == 1) {

            processArray[i].remainingTime = processArray[i].remainingTime - elapsedTime;
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
	return NULL;
}

void substituteProcess(struct process *new_p, struct process *old_p, double cpu_time_used) {

    printf("Vai bloquear processo de remainingTime %f\n", old_p->remainingTime);
	sem_wait(&old_p->semaphore);
    old_p->running = 0;

    startThread(new_p, cpu_time_used);
    printf("Start processo de remainingTime %f\n", new_p->remainingTime);
    printf("Bloqueou processo de remainingTime %f\n", old_p->remainingTime);

/*
	if (new_p->created == 0) {

		createThread(new_p, cpu_time_used);
        startThread(new_p);
        printf("Substitute criando processo de remainingTime %f\n", new_p->remainingTime);

	}
	else {
	
        printf("Vai startar o processo de remainingTime %f\n", new_p->remainingTime);
        sem_post(&new_p->semaphore); 
	}
*/
}

void firstComeFirstServed() {



    
    double cpu_time_used;
    int k, runningThreads, killedProcesses, delta, m;
    clock_t start, end;
    k = runningThreads = killedProcesses = m = 0;
    printf("\nFIRST COME FIRST SERVED\n");
    start = clock();

    qsort(processArray, numThreads,sizeof(struct process),compare_t0);
    printArray(processArray);

    while (killedProcesses < numThreads) {

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        if (k < numThreads && cpu_time_used >= processArray[k].t0) {

            
            if (createThread(&processArray[k], cpu_time_used) == 0) {
                printProcessInfo(processArray[k]);
                k++;
            }
            else abort();
        }

        if (runningThreads < cores) {

            if (m < k) {
                startThread(&processArray[m], cpu_time_used);
                runningThreads++;
                m++;
            }
        }

        if (runningThreads > 0) {

            delta = killProcesses(cpu_time_used);
            runningThreads -= delta;
            killedProcesses += delta;
        }
    }
}

void shortestJobFirst() {




    double cpu_time_used;
    int k, killedProcesses, runningThreads, delta;
    clock_t start, end;
    killedProcesses = runningThreads = 0;
    start = clock();
    printf("\nSHORTEST JOB FIRST\n");
    qsort(processArray, numThreads,sizeof(struct process),compare_dt);
    printArray(processArray);

    while (killedProcesses < numThreads) {

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        k = 0;

        while (k < numThreads) {

            if (cpu_time_used >= processArray[k].t0 && processArray[k].finished == 0 && processArray[k].running == 0) {

                if (processArray[k].created  == 0) {

                        if (createThread(&processArray[k], cpu_time_used) == 0) {

                            printTime(cpu_time_used);
                            printProcessInfo(processArray[k]);
                        }
                        else abort();
                }

                if (runningThreads < cores) { 

                   startThread(&processArray[k], cpu_time_used);
                   runningThreads++;   
                }
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

/* remaining time = dt - (cpu_time - startTime)*/

void shortestRemainingTime() {



    clock_t start, end;
    double cpu_time_used;
    int k, killedProcesses, runningThreads, delta;
    struct process *newProcess, *longest;
    double elapsedTime;  
    killedProcesses = runningThreads = 0;
    printf("\nSHORTEST REMAINING TIME\n");
    start = clock();

    qsort(processArray, numThreads,sizeof(struct process),compare_remainingTime);
    printArray(processArray);

    while (killedProcesses < numThreads) {

        end = clock();
        elapsedTime = cpu_time_used;
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        elapsedTime = cpu_time_used - elapsedTime;
        k = 0;
        /*updateRemainingTimes(cpu_time_used);*/
        updateElapsedTimes(elapsedTime);

        while (k < numThreads) { 
            if (cpu_time_used >= processArray[k].t0 && processArray[k].finished == 0 && processArray[k].running == 0) {
                
                if (processArray[k].created  == 0) {

                        if (createThread(&processArray[k], cpu_time_used) == 0) {

                            printTime(cpu_time_used);
                            printProcessInfo(processArray[k]);
                        }
                        else abort();
                }


                if (runningThreads < cores) { 

                   startThread(&processArray[k], cpu_time_used);
                   runningThreads++;   
                }
                
              
                else {   /*printf("Tentar trocar processo em %f\n", cpu_time_used);*/

                	newProcess = &processArray[k];
                    /*printf("%f\n", cpu_time_used);
                    printf("antes\n");
    		    printArray(processArray);*/
                    qsort(processArray, numThreads,sizeof(struct process),compare_remainingTime);
                    /*printf("depois\n");
                    printArray(processArray);*/
    		    longest = selectTheLongestRemainingTime();
                    /*printf("remaining time do longest: %f\n", longest->remainingTime);
                    printf("remain time do novo: %f\n", newProcess->remainingTime);*/
            		if (longest != NULL && newProcess->remainingTime < longest->remainingTime) {
                        printf("Novo processo tem que entrar no tempo %f\n", cpu_time_used);
            			substituteProcess(newProcess, longest, cpu_time_used);
            		}
        		/*printProcessInfo(*newProcess);*/
        	   }
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

int main() {

    int i;
    numThreads = 5;
    cores = 1;
    readInput();

     
    
    /*mallocProcessArray();
    
    shortestRemainingTime();*/

    /*joinThreads();*/


  exit(0);
}
