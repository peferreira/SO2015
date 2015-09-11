#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>

#define MAX 10000000

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
    int justRun;
    sem_t semaphore;
};

double t0[] = {3,4,5,6,7};
int p[] = {10,4,7,2,5};
double dt[] = {9,7,8,10,5};
double deadline[] = {44,55,22,11,33};
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
  i = 0;

  while (!proc->finished)  {

    sem_wait(&proc->semaphore);
    /* semáforo usado para bloquear o processo */
    //if (i==0) printf("Thread t0 %f passa pelo semaforo\n", proc->t0);

    i++;
    if (i == MAX) {

        printf("Rodando %f\n", proc->t0);
        i = 0;
    }
      
    sem_post(&proc->semaphore);
  }
  
  printf("\tthread t0: %f sai do loop\n", proc->t0);

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
      return 1;
   else if (elem1->p > elem2->p)
      return -1;
   else
      return 0;
}

void printArray(struct process *array) {

    int i;
    printf("Array:\n");
    for (i = 0; i < numThreads; i++) 
        printf("t0 %f dt %f p %d deadline %f\n", array[i].t0, array[i].dt, array[i].p, array[i].deadline);
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
        processArray[i].justRun = 0;
        sem_unlink("semaphore");
        sem_init(&(processArray[i].semaphore), 0, 0);
        //processArray[i].name = name[i];
        processArray[i].thread = malloc(sizeof(pthread_t));
    }
/*
    for (i = 0; i < numThreads; i++) {

        printf("Process %d\n", i);
        printf("t0 %f\n", processArray[i].t0);
        printf("dt %f\n", processArray[i].dt);
        printf("deadline %f\n", processArray[i].deadline);
        printf("p %d\n", processArray[i].p);
        printf("running? %d\n", processArray[i].running);
        //printf("%s\n", processArray[i].name);
    } */
}

void printProcessInfo(struct process p) {

    printf("t0 : %f\t", p.t0);
    printf("dt: %f\n", p.dt);
    printf("remaining time: %f\n", p.remainingTime);
    //printf("start time: %f\n", p.startTime);
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
    
    printf("Startando thread t0 %f\n", p->t0);
    sem_post(&(p->semaphore));
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
                      //sem_wait(&processArray[i].semaphore);
                      //if(pthread_kill(*(processArray[i].thread), SIGCHLD)== 0){
                        printf("Thread de t0 %f morreu em %f\n", processArray[i].t0, cpu_time_used);
                        count++;
                        processArray[i].running = 0;
                        processArray[i].finished = 1;
                        sem_destroy(&processArray[i].semaphore);
                
                }
         }
    }

    return count;
}

int killThisProcess(struct process *p, double cpu_time_used) {

	printf("Thread de t0 %f morreu em %f\n", p->t0, cpu_time_used);
	p->running = 0;
	p->finished = 1;
	sem_destroy(&p->semaphore);
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
}

struct process *selectTheLatestDeadlineProcess() {

    int i;

    for (i = numThreads-1; i >= 0; i--) {
        if (processArray[i].running == 1) {

            return &processArray[i];
        }
    }
}

void substituteProcess(struct process *new_p, struct process *old_p, double cpu_time_used) {

    printf("Vai parar processo de t0 %f e remainingTime %f\n", old_p->t0, old_p->remainingTime);
	sem_wait(&(old_p->semaphore));
    old_p->running = 0;

    startThread(new_p, cpu_time_used);
    printf("Start processo de remainingTime %f\n", new_p->remainingTime);
    printf("Bloqueou processo de t0 %f e remainingTime %f\n", old_p->t0, old_p->remainingTime);
}

void firstComeFirstServed() {

    printf("\nFIRST COME FIRST SERVED\n");

    clock_t start, end;
    double cpu_time_used;
    int i, k, runningThreads, killedProcesses, delta, m;
    k = runningThreads = killedProcesses = m = 0;

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
                printf("%d threads running...\n", runningThreads);
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

// remaining time = dt - (cpu_time - startTime)

void shortestRemainingTime() {

    printf("\nSHORTEST REMAINING TIME\n");

    clock_t start, end;
    double cpu_time_used;
    int i, k, killedProcesses, runningThreads, delta;
    killedProcesses = runningThreads = 0;
    struct process *newProcess, *longest;
    double elapsedTime;

    start = clock();

    qsort(processArray, numThreads,sizeof(struct process),compare_remainingTime);
    printArray(processArray);

    while (killedProcesses < numThreads) {

        end = clock();
        elapsedTime = cpu_time_used;
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        elapsedTime = cpu_time_used - elapsedTime;
        k = 0;
        //updateRemainingTimes(cpu_time_used);
        updateElapsedTimes(elapsedTime);
        //qsort(processArray, numThreads,sizeof(struct process),compare_remainingTime);


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

                	printf("\nTem core disponivel para processo t0 %f\n", processArray[k].t0);
                   startThread(&processArray[k], cpu_time_used);
                   runningThreads++;
                   printf("%d thread(s) running\n", runningThreads);   
                }
                
              
                else {   //printf("Tentar trocar processo em %f\n", cpu_time_used);

                	newProcess = &processArray[k];
                    //printf("%f\n", cpu_time_used);
                    //printf("antes\n");
            		//printArray(processArray);
                    qsort(processArray, numThreads,sizeof(struct process),compare_remainingTime);
                    //printf("depois\n");
                    //printArray(processArray);
            		longest = selectTheLongestRemainingTime();
                    //printf("remaining time do longest: %f\n", longest->remainingTime);
                    //printf("remain time do novo: %f\n", newProcess->remainingTime);
            		if (newProcess->remainingTime < longest->remainingTime) {
                        printf("Novo processo tem que entrar no tempo %f\n", cpu_time_used);
            			substituteProcess (newProcess, longest, cpu_time_used);
            		}
        		//printProcessInfo(*newProcess);
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


void roundRobin() {
    
    printf("\nROUND ROBIN\n");
    
    clock_t start, end;
    double cpu_time_used;
    int i, k, killedProcesses, runningThreads, delta, firstToBeStarted;
    killedProcesses = runningThreads = 0;
    double quantum, elapsedTime, timer;
    struct Node *listNode;
    
    cpu_time_used = elapsedTime = firstToBeStarted = 0;
    quantum = 5.0;
    
    start = clock();
    
    qsort(processArray, numThreads,sizeof(struct process), compare_t0);
    printArray(processArray);
    
    while (killedProcesses < numThreads) {
        
        end = clock();
        elapsedTime = cpu_time_used;
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        elapsedTime = cpu_time_used - elapsedTime;
        //timer = timer + elapsedTime;
        
        updateElapsedTimes(elapsedTime);
        
        k = 0;
        while (k < numThreads) {
            if (cpu_time_used >= processArray[k].t0 && processArray[k].finished == 0) {
                
                /* se o processo não foi criado ainda, cria o processo */
                if (processArray[k].created  == 0) {
                    
                    printf("\t vai criar um processo t0 %f\n", processArray[k].t0);
                    if (createThread(&processArray[k], cpu_time_used) == 0) {
                        
                        printTime(cpu_time_used);
                        printProcessInfo(processArray[k]);
                    }
                    else abort();
                }
            }
            k++;
        }

        /* se tem processos no vetor e cores disponíveis, roda os processos enquanto houver cores */
         k = firstToBeStarted;
        while (cpu_time_used > processArray[k].t0 && k < numThreads && runningThreads < cores) {
            
            if (processArray[k].finished == 0 && processArray[k].running == 0) {
                printf("Vou startar o processo de t0 %f\n", processArray[k].t0);
                startThread(&processArray[k], cpu_time_used);
                runningThreads++;
                printf("%d threads running...\n", runningThreads);
            }
            k++;
        }
        k = 0;
        while (cpu_time_used > processArray[k].t0 && k < firstToBeStarted && runningThreads < cores) {
            if (processArray[k].finished == 0 && processArray[k].running == 0) {
                printf("Vou startar o processo de t0 %f\n", processArray[k].t0);
                startThread(&processArray[k], cpu_time_used);
                runningThreads++;
                printf("%d threads running...\n", runningThreads);
            }
            k++;
        }
        

        /* se um processo rodou por um quantum, o processo deve parar */
        k = 0;
        while (k < numThreads) {

            if (processArray[k].running == 1) {

                //printf("\t t0 %f running\n", processArray[k].t0);
                
                //if (cpu_time_used >= 5) { printf("Clock %f\n", cpu_time_used); processArray[k].startTime;}


                if ((cpu_time_used - processArray[k].startTime) >= quantum) {

                	if (processArray[k].remainingTime <= 0) {

                		printf("Processo t0 %f deve morrer: start time %f, clock %f\n", processArray[k].t0, processArray[k].startTime, 
                			cpu_time_used);
                    	killThisProcess(&processArray[k], cpu_time_used);
                    	killedProcesses++;
                    	runningThreads--;
                    }
                    else {
                    	printf("Processo t0 %f deve parar: start time %f, clock %f\n", processArray[k].t0, processArray[k].startTime, 
                    		cpu_time_used);
                		sem_wait(&(processArray[k].semaphore));  /* paro o processo */
                		processArray[k].running = 0;
                		runningThreads--;
                		printf("%d threads running...\n", runningThreads);
                    }
                    firstToBeStarted = (k+1)%numThreads;
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

void priorityScheduling() {
    
    printf("\nPRIORITY SCHEDULING\n");
    
    clock_t start, end;
    double cpu_time_used;
    int k, killedProcesses, runningThreads, delta;
    double quantum, elapsedTime, timer;
    struct Node *listNode;
    
    killedProcesses = runningThreads = 0;
    cpu_time_used = elapsedTime = 0;
    quantum = 5.0;
    
    start = clock();
    
    qsort(processArray, numThreads,sizeof(struct process), compare_p);
    printArray(processArray);
    
    while (killedProcesses < numThreads) {
        
        end = clock();
        elapsedTime = cpu_time_used;
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        elapsedTime = cpu_time_used - elapsedTime;
        
        updateElapsedTimes(elapsedTime);
        
        k = 0;
        while (k < numThreads) { 

            if (cpu_time_used >= processArray[k].t0 && processArray[k].finished == 0) { 
                
                /* se o processo não foi criado ainda, cria o processo */
                if (processArray[k].created  == 0) {
                    
                    printf("\t vai criar um processo t0 %f\n", processArray[k].t0);
                    if (createThread(&processArray[k], cpu_time_used) == 0) {
                        
                        printTime(cpu_time_used);
                        printProcessInfo(processArray[k]);
                    }
                    else abort();
                }
            }
            k++;
        }

        /* se tem processos no vetor e cores disponíveis, roda os processos que não saíram na última rodada enquanto houver cores */
        k = 0;
        while (k < numThreads && runningThreads < cores) {
            
            if (cpu_time_used > processArray[k].t0 && processArray[k].finished == 0 && processArray[k].running == 0 && processArray[k].justRun == 0) {
                printf("Vou startar o processo de t0 %f\n", processArray[k].t0);
                startThread(&processArray[k], cpu_time_used);
                runningThreads++;
                printf("%d threads running...\n", runningThreads);
            }
            k++;
        }

        /* se ainda tiver cores, rodo os processos que estavam rodando na última */
        k = 0;
        while (k < numThreads && runningThreads < cores) {
            if (cpu_time_used > processArray[k].t0 && processArray[k].finished == 0 && processArray[k].running == 0 && processArray[k].justRun == 1) {
                printf("Vou startar o processo de t0 %f\n", processArray[k].t0);
                startThread(&processArray[k], cpu_time_used);
                runningThreads++;
                printf("%d threads running...\n", runningThreads);
            }
            k++;
        }

        k = 0;
        while (k < numThreads) {

            processArray[k].justRun = 0;
            k++;
        }

 		/* se um processo rodou por um quantum, o processo deve parar */
        k = 0;
        while (k < numThreads) {

            if (processArray[k].running == 1) {

                if ((cpu_time_used - processArray[k].startTime) >= quantum) {

                	if (processArray[k].remainingTime <= 0) {

                		printf("Processo t0 %f deve morrer: start time %f, clock %f\n", processArray[k].t0, processArray[k].startTime, 
                			cpu_time_used);
                    	killThisProcess(&processArray[k], cpu_time_used);
                    	killedProcesses++;
                    	runningThreads--;
                    }
                    else {
                    	printf("Processo t0 %f deve parar: start time %f, clock %f\n", processArray[k].t0, processArray[k].startTime, 
                    		cpu_time_used);
                		sem_wait(&(processArray[k].semaphore));  /* paro o processo */
                		processArray[k].running = 0;
                		processArray[k].justRun = 1;
                		runningThreads--;
                		printf("%d threads running...\n", runningThreads);
                    }
                }
           }
           k++;
        }

        /* se um processo rodou por um quantum, o processo deve parar */
        /*
        k = 0;
        while (k < numThreads) {

            if (processArray[k].running == 1) {

                if ((cpu_time_used - processArray[k].startTime) >= quantum) {

                    printf("Processo t0 %f deve parar: start time %f, clock %f\n", processArray[k].t0, 
                    processArray[k].startTime, cpu_time_used);
                    sem_wait(&(processArray[k].semaphore));  
                    processArray[k].running = 0;
                    processArray[k].justRun = 1;
                    runningThreads--;
                    printf("%d threads running...\n", runningThreads);
                    
                    if (processArray[k].remainingTime <= 0) {
                            processArray[k].finished = 1;
                            killedProcesses++;
                        }
                }
            }
            k++;
        } */

        if (runningThreads > 0) {
            
            delta = killProcesses(cpu_time_used);
            runningThreads -= delta;
            killedProcesses += delta;
        }
    }
}

void earliestDeadlineFirst() {

    printf("\nEARLIEST DEADLINE FIRST\n");

    clock_t start, end;
    double cpu_time_used;
    int i, k, killedProcesses, runningThreads, delta;
    killedProcesses = runningThreads = 0;
    struct process *newProcess, *latestDeadlineProcess;
    double elapsedTime;

    start = clock();

    qsort(processArray, numThreads,sizeof(struct process),compare_deadline);
    printArray(processArray);

    while (killedProcesses < numThreads) {

        end = clock();
        elapsedTime = cpu_time_used;
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        elapsedTime = cpu_time_used - elapsedTime;
        k = 0;
        //updateRemainingTimes(cpu_time_used);
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
                   printf("%d thread(s) running...\n", runningThreads);   
                }

                else {   


                    newProcess = &processArray[k];
                    //printf("%f\n", cpu_time_used);
                    //printf("antes\n");
                    //printArray(processArray);
                    //printf("depois\n");
                    //printArray(processArray);
                    latestDeadlineProcess = selectTheLatestDeadlineProcess();
                    //printf("remaining time do longest: %f\n", longest->remainingTime);
                    //printf("remain time do novo: %f\n", newProcess->remainingTime);
                    if (newProcess->deadline < latestDeadlineProcess->deadline) {
                        printf("Novo processo tem que entrar no tempo %f\n", cpu_time_used);
                        substituteProcess (newProcess, latestDeadlineProcess, cpu_time_used);
                    }
                //printProcessInfo(*newProcess);
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

    mallocProcessArray();
    
    shortestRemainingTime();

    //joinThreads();
    

    /*double t0[] = {3,4,5,6,7};
      double dt[] = {9,7,8,10,5};
          
    int p[] = {10,4,7,2,5};
    double deadline[] = {44,55,22,11,33};
    char name[] = {'a','b','c','d','e'};*/

  exit(0);
}


/*************************************** ROUND ROBIN ANTIGO ************************************
void roundRobin() {
    
    printf("\nROUND ROBIN\n");
    
    clock_t start, end;
    double cpu_time_used;
    int i, k, killedProcesses, runningThreads, delta, freeCores, queuedProcesses;
    killedProcesses = runningThreads = 0;
    struct process *newProcess;
    double quantum, elapsedTime, timer;
    struct Node *listNode;
    
    cpu_time_used = elapsedTime = timer = 0;
    quantum = 5.0;
    
    start = clock();
    
    qsort(processArray, numThreads,sizeof(struct process), compare_t0);
    printArray(processArray);
    
    while (killedProcesses < numThreads) {
        
        end = clock();
        elapsedTime = cpu_time_used;
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        elapsedTime = cpu_time_used - elapsedTime;
        timer = timer + elapsedTime;
        
        k = 0;
        //updateRemainingTimes(cpu_time_used);
        updateElapsedTimes(elapsedTime);
        
        while (k < numThreads) {
            if (cpu_time_used >= processArray[k].t0 && processArray[k].finished == 0 && processArray[k].running == 0) {
                
                
                if (processArray[k].created  == 0) {
                    
                    if (createThread(&processArray[k], cpu_time_used) == 0) {
                        
                        printTime(cpu_time_used);
                        printProcessInfo(processArray[k]);
                        enqueue(&processArray[k]);
                    }
                    else abort();
                }
                
                if (runningThreads < cores && !emptyQueue()) {
                    
                    printf("Tem core disponivel e processo na fila\n");
                    printQueue();
                    
                    while (runningThreads < cores && !emptyQueue()) {
                        
                        newProcess = firstOfTheQueue()->proc;
                        if (newProcess != NULL) {
                            startThread(&processArray[k], cpu_time_used);
                            runningThreads++;
                            printf("%d threads running...\n", runningThreads);
                        } else printf("Null process\n (lista vazia)");
                    }
                }
                
                if (timer >= quantum) {
                    
                    timer = 0; 
                    printf("Trocar processos. Clock: %f Timer: %f\n", cpu_time_used, timer);
                    
                    if (runningThreads > 0) {
                        
                        i = 0;
                        while (i < runningThreads) {
                            
                            newProcess = firstOfTheQueue()->proc;
                            if (newProcess != NULL) {
                                
                                printf("Vou parar o processo de t0 %f remaining %f\n", newProcess->t0, newProcess->remainingTime);
                                sem_wait(&(newProcess->semaphore));  
                                newProcess->running = 0;
                                
                                runningThreads--;
                                printf("%d threads running...\n", runningThreads);
                                dequeue();
                                
                                if (newProcess->remainingTime > 0) {
                                    enqueue(newProcess);
                                }
                                else {
                                    printf("\n\n\t\tnao volta pra fila t0 %f pois já terminou\n\n\n", newProcess->t0);
                                    newProcess->running = 0;
                                    newProcess->finished = 1;
                                    killedProcesses++;
                                }
                                i++;
                            }
                        }
                        
                        i = 0;
                        listNode = firstOfTheQueue();
                        while (i < cores && i < queueSize()) {
                            
                            printf("Vou startar o processo de t0 %f\n", listNode->proc->t0);
                            startThread(listNode->proc, cpu_time_used);
                            runningThreads++;
                            printf("%d threads running...\n", runningThreads);
                            listNode = listNode->next;
                            i++;
                        }
                    }
                }
            }
            k++;
        }
        
        if (runningThreads > 0) {
            
            delta = killProcesses(cpu_time_used);
            runningThreads -= delta;
            killedProcesses += delta;
            
            if (delta > 0) {
                
                printf("\nTirar %d processos da fila\t fila tamanho %d\n", delta, queueSize());
                printf("%d killed processes\n", killedProcesses);
                dequeueKilledProcesses(delta);
                printf("\tAgora tem %d na fila\n", queueSize());
            }
        }
    }
} */

/*

struct Node {
    struct process *proc;
    struct Node* next;
};

struct Node *front = NULL;
struct Node *rear = NULL;

void enqueue(struct process *p) {
    
    printf("vou colocar na fila processo t0 %f\n", p->t0);
    
    struct Node *temp;
    temp = (struct Node*) malloc(sizeof(struct Node));
    temp->proc = p;
    temp->next = NULL;
    
    if(front == NULL && rear == NULL){
        front = rear = temp;
        return;
    }
    rear->next = temp;
    rear = temp;
}

void dequeue() {
    
    printf("vou tirar da fila processo t0 %f\n", front->proc->t0);
    
    struct Node *temp = front;
    if(front == NULL) 
        return;
    
    if(front == rear) {
        front = rear = NULL;
    }
    else {
        front = front->next;
    }
    free(temp);
}

struct Node *firstOfTheQueue() {
    
    if(front == NULL) {
        return NULL;
    }
    return front;
}

int emptyQueue() {
    
    if(front == NULL)
        return 1;
    else return 0;
}

int queueSize() {
    
    int size;
    struct Node *temp;
    temp = front;
    
    size = 0;
    
    while (temp != NULL) {
        size++;
        temp = temp->next;
    }
    
    return size;
}

void printQueue() {
    
    struct Node *temp;
    temp = front;
    
    printf("\tFila:\n");
    while (temp != NULL) {
        printf("\tprocesso t0 %f\n", temp->proc->t0);
        temp = temp->next;
    }
}

void dequeueKilledProcesses(int delta) {
    
    struct Node *temp, *q, *r;
    
    temp = (struct Node*) malloc(sizeof(struct Node));
    
    temp = firstOfTheQueue();
    q = temp->next;
    r = q->next;
    
    printf("Removendo processos da fila\n");
    
    while (delta > 0 && temp != NULL) {
        
        if (temp->proc->finished == 1) {
            
            if (temp == front) {
                temp = temp->next;
                printf("\tVai remover processo do front t0 %f\n", front->proc->t0);
                free(front);
                front = temp;
                q = front->next;
            }
            else {
                if (q->proc->finished == 1) {
                    printf("\tVai remover processo front->next t0 %f\n", q->proc->t0);
                    free(q);
                    temp = r;
                    if (temp->next != NULL) {
                        q = temp->next;
                        if (q->next != NULL) {
                            r = q->next;
                        }
                    }
                    else {
                        q = NULL; r = NULL;
                    }
                }
            }
            printQueue();
            delta--;
        }
        else {
            printQueue();
            printf("proc t0 %f, finished %d\n", temp->proc->t0, temp->proc->finished);
            if (temp == NULL) {
                printf("processo nulo\n");
            }
            printf("\t\txxxxxxxxxxx\n");
            temp = temp->next;
            if (temp != NULL) {
                q = temp->next;
                if (q != NULL) {
                    r = q->next;
                }
            }
            else {
                q = NULL; r = NULL;
            }
        }
    }
} */