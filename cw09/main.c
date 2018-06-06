/* ====INCLUDE BLOCK==== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* ====END OF INCLUDE BLOCK==== */


/* ====DEFINE BLOCK==== */
#define ERROR_MSG(format, ...) { printf(format, ##__VA_ARGS__); exit(-1);}

/* ====END OF DEFINE BLOCK==== */

/* enum */
enum data_status{
    ISPLACE = 0, FULL = 1
};

enum which_searching{
    EQUAL = 0, SMALLER = 1, GREATER = 2
};


/* shared memory struct */
struct shared_memory_data{
    char** dataArray;
    int manufacturerPosition;
    int customerPosition;
    enum data_status status;
};

/* thread arg struct */
struct thread_arg{
    int P, K, N, L, nk;
    unsigned char writingFlag;       // 1 - opisowy, 0 - uproszczony
    enum which_searching searching;
};


/* ====GLOBAL VARIABLES BLOCK==== */
struct shared_memory_data data;
static FILE* dataFile;
pthread_mutex_t mutex;
pthread_cond_t cond;

/* ====END OF GLOBAL VARIABLES BLOCK==== */


/* ====FUNCTION DECLARATIONS BLOCK==== */
void init(char* fileName, struct thread_arg* tArgs);
void at_exit(void);
void threads(struct thread_arg* tArgs);
void* cus_function(void* arg);
void* man_function(void* arg);

/* ====END OF FUNCTION DECLARATION BLOCK==== */

void fill_data(struct thread_arg* a, struct thread_arg b){
    a->N = b.N;
    a->K = b.K;
    a->L = b.L;
    a->nk = b.nk;
    a->searching = b.searching;
    a->P = b.P;
    a->writingFlag = b.writingFlag;
}


/* ====FUNCTIONS==== */
int main(int argc, char* argv[]){
    if (argc < 2)
        ERROR_MSG("Too few arguments\n");

    struct thread_arg tArgs;

    init(argv[1], &tArgs);

    pthread_t man[tArgs.P], cus[tArgs.K];
   // struct thread_arg mData[tArgs.P], cData[tArgs.K];

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


    printf("%i --\n", tArgs.P);
    for (int i = 0; i < tArgs.P; ++i) {
        printf("%i ss\n", i);

        if (pthread_create(&man[i], &attr, man_function, (void *) &tArgs) == -1)
        ERROR_MSG("Couldn't create a thread\n");
    }
    for (int i = 0; i < tArgs.K; ++i) {

        if (pthread_create(&cus[i], &attr, cus_function, (void *) &tArgs) == -1)
        ERROR_MSG("Couldn't create a thread\n");
    }

    pthread_attr_destroy(&attr);

    for (int i = 0; i < tArgs.P; ++i) {
        pthread_join(man[i], NULL);
    }
    for (int i = 0; i < tArgs.K; ++i) {
        pthread_join(cus[i], NULL);
    }
}


//P, K, N, name, L, trybszukania, trybwypisywania, nk
void init(char* fileName, struct thread_arg* tArgs){
    FILE *pFILE = fopen(fileName, "r");

    char* buf = NULL;
    size_t len;


    if (getline(&buf, &len, pFILE) == -1)
        ERROR_MSG("No P\n");
    tArgs->P = atoi(buf);

    if (getline(&buf, &len, pFILE) == -1)
        ERROR_MSG("No K\n");
    tArgs->K = atoi(buf);

    if (getline(&buf, &len, pFILE) == -1)
        ERROR_MSG("No N\n");
    tArgs->N = atoi(buf);

    if (getline(&buf, &len, pFILE) == -1)
        ERROR_MSG("No dataFile\n");
    buf[strlen(buf)] = '\0';
    //if ((dataFile = fopen(buf, "r")) == -1)
      //  ERROR_MSG("hcuasdasd\n");

    if (getline(&buf, &len, pFILE) == -1)
        ERROR_MSG("No L\n");
    tArgs->L = atoi(buf);

    if (getline(&buf, &len, pFILE) == -1)
    ERROR_MSG("No searching_type\n");
    //buf[strlen(buf)] = '\0';
    switch (buf[0]){
        case '=':
            tArgs->searching = EQUAL;
            break;
        case '>':
            tArgs->searching = GREATER;
            break;
        case '<':
            tArgs->searching = SMALLER;
            break;
        default:
            ERROR_MSG("Wrong searching type\n");
    }

    if (getline(&buf, &len, pFILE) == -1)
        ERROR_MSG("No writingFlag\n");
    tArgs->writingFlag = atoi(buf);

    if (getline(&buf, &len, pFILE) == -1)
        ERROR_MSG("No nk\n");
    tArgs->nk = atoi(buf);

    /* initializing main array */
    data.dataArray = (char**)calloc(tArgs->N, sizeof(char*));
    data.customerPosition = 0;
    data.manufacturerPosition = 0;
    data.status = ISPLACE;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    fclose(pFILE);
}


/* ====MANUFACTURER BLOCK==== */
void parse_line(int whichBlock){
    char* tmp;
    size_t size;
    int len;
   // printf("MUTEX1\n");

   // printf("MUTEX2\n");
    FILE* huj = fopen("pan_tadeusz.txt", "r");
    getline(&tmp, &size, huj);
    fclose(huj);

    len = strlen(tmp);
    printf("%s\n", tmp);

   // printf("MUTEX3\n");
    if(tmp[len-1] == '\n') {
        tmp[len-1] = '\0';
        len--;

     //   printf("MUTEX4\n");
    }


    data.dataArray[whichBlock] = calloc(len, sizeof(char));
    strcpy(data.dataArray[whichBlock], tmp);
}

int is_a_free_slot(int i, int N){
    //int i = data.manufacturerPosition;
    do {
        if (i == N)
            i = 0;

        if (data.dataArray[i] == NULL || data.dataArray[i][0] == 0) {
            printf("=======i: %i=======\n", i);
            return i;             // found an empty slot
        }
        i++;
    } while (i != data.manufacturerPosition);

    return -1;
}

void* man_function(void* arg){
    struct thread_arg* my_data;
    my_data = (struct thread_arg*) arg;

    struct timespec start;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &start);          // time in which thread started

    while (1) {
        pthread_mutex_lock(&mutex);


        int whichBlock;
        // waiting for an empty pos
        while ((whichBlock = is_a_free_slot(data.manufacturerPosition, my_data->N)) == -1){
            clock_gettime(CLOCK_REALTIME, &now);

            // time has ended
            if (my_data->nk > 0 && now.tv_sec - start.tv_sec > my_data->nk){
                pthread_mutex_unlock(&mutex);
                return (void*) 0;
            }

            pthread_cond_wait(&cond, &mutex);
        }


        // putting line into data array
        parse_line(whichBlock);
        data.manufacturerPosition = whichBlock;
        if(data.status == FULL){
            data.status = ISPLACE;
            pthread_cond_broadcast(&cond);

        }


        pthread_mutex_unlock(&mutex);
    }
}
/* ====END OF MANUFACTURER BLOCK==== */


/* ====CUSTOMER BLOCK==== */
int first_not_empty_block(int i, int N){
    do {
        if (i == N) {
            i = 0;
            if (i == data.customerPosition)
                break;
        }

       // printf("i: %i\n", i);
        if (data.dataArray[i] != NULL) {
            printf("HUJEX1\n");
            return i;             // found an not empty slot
        }
        i++;
    } while (1);

    return -1;
}

void write_string(int whichBlock, int searching, int maxSize){
    char tmp[strlen(data.dataArray[whichBlock])+1];
    strcpy(tmp, data.dataArray[whichBlock]);
    free(data.dataArray[whichBlock]);
    data.dataArray[whichBlock] = NULL;

    printf("wypisywanie:\n");
    switch (searching){
        case EQUAL:
            if (strlen(tmp) == maxSize)
                printf("%s\n", tmp);
            break;
        case GREATER:
            if (strlen(tmp) > maxSize)
                printf("%s\n", tmp);
            break;
        case SMALLER:
            if (strlen(tmp) < maxSize)
                printf("%s\n", tmp);
            break;
        default:
            printf("ERROR\n");
    }

    //free(data.dataArray[whichBlock]);
    //free(tmp);

}

void* cus_function(void *arg){
    struct thread_arg* my_data;
    my_data = (struct thread_arg*) arg;
    printf("customer %i!!\n", my_data->N);

    struct timespec start, now;
    clock_gettime(CLOCK_REALTIME, &start);          // time in which thread started

    while (1){
        pthread_mutex_lock(&mutex);

        int whichBlock;


        // waiting for an empty pos
        while ((whichBlock = first_not_empty_block(data.customerPosition, my_data->N)) == -1){
            clock_gettime(CLOCK_REALTIME, &now);

           // printf("kurwa:%ld\n", now.tv_sec);
            // check if the time has ended
            if (my_data->nk > 0 && now.tv_sec - start.tv_sec > my_data->nk){
                pthread_mutex_unlock(&mutex);
                return (void*) 0;
            }

            pthread_cond_wait(&cond, &mutex);
        }

            printf("%i \n", whichBlock);
            // deleting a block
            write_string(whichBlock, my_data->searching, my_data->L);
            printf("hujex2\n");
            data.customerPosition = whichBlock;
            data.action = DELETED;
            pthread_cond_broadcast(&cond);


        pthread_mutex_unlock(&mutex);
    }

}

/* ====END OF CUSTOMER BLOCK==== */
