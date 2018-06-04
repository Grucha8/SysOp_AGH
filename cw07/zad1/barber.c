#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <time.h>

#include "helper.h"

/* functions declarations */
int is_queue_empty(int *queue);
void release_sem();
void get_sem();

/* global varaibles */
int max_number_of_clients = 0;
int shared_memory_id;
int semaphore_id;
struct timespec t;

/* handlers */
void termint_handler(int _signum){
    printf("BARBER: shutting down");
    exit(0);
}

void exit_handler(){
    semctl(semaphore_id, 0, IPC_RMID);

    shmctl(shared_memory_id, IPC_RMID, NULL);
}

//argv[1] - number of max clients
int main(int argc, char* argv[]){
    atexit(exit_handler);

    signal(SIGTERM, termint_handler);
    signal(SIGINT, termint_handler);

    if(argc != 2)
        ERROR_MSG("BARBER: Wrong number of arguments\n");

    max_number_of_clients = atoi(argv[1]);

    if(max_number_of_clients < 1)
        ERROR_MSG("BARBER: The amount of max clients must be greater than 0\n");

    char* pathname;

    if ((pathname = getenv("HOME")) == NULL)
        ERROR_MSG("BARBER: couldn't get the pathname\n");

    /* creating the shared memory */
    key_t key;
    if ((key = ftok(pathname, BARBER_ID)) < 0)
        ERROR_MSG("BARBER: couldn't generate barber queue key\n");

    if((shared_memory_id = shmget(key, sizeof(struct Barbershop), 0777 | IPC_CREAT)) == -1)
        ERROR_MSG("BARBER: Couldn't make shared memory\n");

    /* getting access to shared memory */
    void *shared_memory;

    if((shared_memory = shmat(shared_memory_id, NULL, 0)) == (void*) -1)
        ERROR_MSG("BARBER: Couldn't get access to shared memory\n");
    barbershop = shmat(shared_memory_id, 0, 0);

    /* creating semaphore */
    if((semaphore_id = semget(key, 1, 0777 | IPC_CREAT)) == -1)
        ERROR_MSG("BARBER: Couldn't make semaphore\n");

    /* getting access to semaphore */
    if(semctl(semaphore_id, 0, SETVAL, 0) == -1)
        ERROR_MSG("BARBER: Couldn't get access to semaphore\n");

    /* initialazing the barbershop struct */
    barbershop->status = SLEEPING;
    barbershop->max_amount_of_clients = max_number_of_clients;
    barbershop->waiting_clients = 0;
    for (int i = 0; i < max_number_of_clients; i++)
        barbershop->queue[i] = 0;

    release_sem();
    /* main loop */
    while (1){
        get_sem();

        if(barbershop->status == WAITING && is_queue_empty(barbershop->queue)){
            clock_gettime(CLOCK_MONOTONIC, &t);
            printf("%ld\tBARBER: No clients avaible, start sleeping\n", t.tv_nsec/1000);
            barbershop->status = SLEEPING;
        }
        else if (barbershop->status == WAITING && !is_queue_empty(barbershop->queue)){
            clock_gettime(CLOCK_MONOTONIC, &t);
            printf("%ld\tBARBER: Inviting client: %d\n", t.tv_nsec/1000, barbershop->queue[0]);
            barbershop->status = WORKING;
            barbershop->current_client = barbershop->queue[0];
        }
        else if(barbershop->status == WORKING){
            clock_gettime(CLOCK_MONOTONIC, &t);
            printf("%ld\tBARBER: Shaving client: %d\n", t.tv_nsec/1000, barbershop->current_client);
            barbershop->status = WAITING;
            printf("%ld\tBARBER: Shaved client %d\n", t.tv_nsec/1000, barbershop->current_client);
            barbershop->current_client = 0;
        }
        else if(barbershop->status == AWEKING){
            clock_gettime(CLOCK_MONOTONIC, &t);
            printf("%ld\tBARBER: I have been awoken!\n", t.tv_nsec/1000);
            barbershop->status = WAITING;
        }

        release_sem();
    }
}

void get_sem() {
    struct sembuf sem;
    sem.sem_num = 0;
    sem.sem_flg = 0;
    sem.sem_op = -1;

    if(semop(semaphore_id, &sem, 1) == -1)
        ERROR_MSG("BARBER: Couldn't get semaphore\n");
}

void release_sem(){
    struct sembuf sem;
    sem.sem_num = 0;
    sem.sem_flg = 0;
    sem.sem_op = 1;

    if(semop(semaphore_id, &sem, 1) == -1)
        ERROR_MSG("BARBER: Couldn't release semaphore\n");
}

int is_queue_empty(int *queue) {

    if (queue[0] == 0)
        return 1;

    return 0;
}
