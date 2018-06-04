#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#include "helper.h"

/* global variables */
int number_of_clients = 0;
int S = 0;
int shared_memory_id = 0;
int semaphore_id = 0;

/* function declarations */
void client();
void get_sem(pid_t pid);
void release_sem(pid_t pid);
int is_queue_full();

//argv[1] = number_of_clients, argv[2] = S
int main(int argc, char* argv[]){

    if (argc < 3)
        ERROR_MSG("CLIENTS: Wrong number of arguments");

    number_of_clients = atoi(argv[1]);
    if(number_of_clients < 1)
        ERROR_MSG("CLIENTS: Must be more than 0 clients\n");

    S = atoi(argv[2]);
    if (S < 1)
        ERROR_MSG("CLIENTS: Must be more than 0 shaves\n");

    /* generating key and getting shared memory */
    char* pathname;
    if ((pathname = getenv("HOME")) == NULL)
        ERROR_MSG("CLIENTS: couldn't get the pathname\n");
    key_t key;
    if ((key = ftok(pathname, BARBER_ID)) < 0)
        ERROR_MSG("CLIENTS: couldn't generate barber queue key\n");
    if((shared_memory_id = shmget(key, sizeof(struct Barbershop), 0777 | IPC_CREAT)) == -1)
        ERROR_MSG("CLIENTS: Couldn't connect to shared memory\n");
    /* getting access to shared memory */
    void *shared_memory;

    if((shared_memory = shmat(shared_memory_id, NULL, 0)) == (void*) -1)
        ERROR_MSG("CLIENTS: Couldn't get access to shared memory\n");
    barbershop = shmat(shared_memory_id, 0, 0);

    /* getting semaphore */
    if((semaphore_id = semget(key, 0, 0)) == -1)
        ERROR_MSG("CLIENTS: Couldn't get semaphore\n");

    /* creating clients */
    for (int i = 0; i < number_of_clients; ++i) {
        if(fork() == 0){
            client();
            exit(0);
        }
    }

    int status = 0;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0);
}

void client() {
    pid_t pid = getpid();
    int how_many_shaves = 0;
    enum client_status status = NEW;

    struct timespec t;

    /* client main loop */
    while (how_many_shaves < S){
        get_sem(pid);

        /* client walks into barbershop */
        if(barbershop->status == SLEEPING) {            //barber is sleeping
            clock_gettime(CLOCK_MONOTONIC, &t);
            printf("%ld\tCLIENT %d: Waking up the barber\n", t.tv_nsec/1000, pid);
            barbershop->status = AWEKING;

            // as we cannot use sleep we must in a while loop release and get the semaphor
            // until the barber will be ready
            while ( 1 ){
                release_sem(pid);
                get_sem(pid);
                if (barbershop->status == WAITING) break;
            }
            status = SHAVING;
            barbershop->current_client = pid;

            clock_gettime(CLOCK_MONOTONIC, &t);
            printf("%ld\tCLIENT %d: I took place\n", t.tv_nsec/1000, pid);

            barbershop->status = WORKING;
        }
        else if (is_queue_full()){                      //the queue is full
            clock_gettime(CLOCK_MONOTONIC, &t);
            printf("%ld\tCLIENT %d: There isn't any place for me\n", t.tv_nsec/1000, pid);
            release_sem(semaphore_id);
            return;
        }
        else if (!is_queue_full()){                     //the queue isnt full
            clock_gettime(CLOCK_MONOTONIC, &t);
            printf("%ld\tCLIENT %d: Entering the queue on position: %d\n", t.tv_nsec/1000, pid, barbershop->waiting_clients);
            barbershop->queue[barbershop->waiting_clients] = pid;
            barbershop->waiting_clients++;
            status = AWAITING;
        }

        /* client is waiting for his shave */
        release_sem(semaphore_id);

        while (status == AWAITING){
            get_sem(semaphore_id);

            if(barbershop->current_client == pid) {
                clock_gettime(CLOCK_MONOTONIC, &t);
                printf("%ld\tCLIENT %d: I took place\n", t.tv_nsec/1000, pid);
                status = SHAVING;

                /* popping first client */
                for (int i = 0; i < barbershop->waiting_clients - 1; ++i) {
                    barbershop->queue[i] = barbershop->queue[i+1];
                }
                barbershop->queue[barbershop->waiting_clients - 1] = 0;
                barbershop->waiting_clients--;

                barbershop->status = WORKING;
            }
            release_sem(semaphore_id);
        }

        while (status == SHAVING){
            get_sem(pid);
            if (barbershop->current_client != pid) {
                how_many_shaves++;
                clock_gettime(CLOCK_MONOTONIC, &t);
                printf("%ld\tCLIENT %d: I have been shaved %d time. Getting back to lobby\n", t.tv_nsec/1000, pid, how_many_shaves);
                status = AWAITING;                      //getting back to the lobby
                barbershop->status = WAITING;
            }
            release_sem(pid);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t);
    printf("%ld\tCLIENT %d: Left the barbershop\n", t.tv_nsec/1000, pid);
}


void get_sem(pid_t pid) {
    struct sembuf sem;
    sem.sem_num = 0;
    sem.sem_flg = 0;
    sem.sem_op = -1;

    if(semop(semaphore_id, &sem, 1) == -1)
        ERROR_MSG("CLIENT %d: Couldn't get semaphore\n", pid);
}

void release_sem(pid_t pid){
    struct sembuf sem;
    sem.sem_num = 0;
    sem.sem_flg = 0;
    sem.sem_op = 1;

    if(semop(semaphore_id, &sem, 1) == -1)
        ERROR_MSG("CLIENT %d: Couldn't release semaphore\n", pid);
}

int is_queue_full(){
    if (barbershop->waiting_clients < barbershop->max_amount_of_clients)
        return 0;

    return 1;
}