#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/times.h>
#include <unistd.h>


#define ERROR_MSG(format, ...) { printf(format, ##__VA_ARGS__); exit(-1);}
#define DELETE_ENDLINE(string) { string[strlen(string)-1] = 0;}
#define MAX_PIXEL 255
#define EPSILION 1e7


/* functions declarations */
void prog_init(int argc, char* argv[]);
void parse_input_image(FILE *pFILE);
void parse_filter_file(FILE *pFILE);
void threads_handler();
void *thread_function(void* arg);
void writing_to_output_file(char* file_name);
double filter_fun(int x, int y);

/* global varaibles */
int Threads;                /* number of threads */
int W, H;                   /* Width, Heigth */
int C;                      /* filter */
int** I;          /* matrix of input image */
double** K;                  /* matrix of filter */
int** J;          /* output image */


/* atexit and signal handlers */
void exit_handler(){
    // free the matrix space
    if(K){
        for (int i = 0; i < C; ++i)
            free(K[i]);

        free(K);
    }
    if(I){
        for (int i = 0; i < W; ++i)
            free(I[i]);

        free(I);
    }
    if(J){
        for (int i = 0; i < W; ++i)
            free(J[i]);
        free(J);
    }
}

/* thread argument struct */
struct thread_data{
    int thread_num;
};


// 1 = watki, 2 = plik wejsciowy, 3 = plik z filtr, 4 = plik wynikowy
int main(int argc, char* argv[]){
    struct timespec start, end;

    prog_init(argc, argv);

    printf("Liczba watkow: %i, Rozmiar filtra: %i\n", Threads, C);

    clock_gettime(CLOCK_REALTIME, &start);
    threads_handler();
    clock_gettime(CLOCK_REALTIME, &end);

    long t;

    if (C == 64)
        t = (end.tv_sec - start.tv_sec);
    else
        t = (end.tv_nsec - start.tv_nsec);
    printf("Czas rzeczywisty: %ld\n", t);

    writing_to_output_file(argv[4]);

}


void writing_to_output_file(char* file_name) {
    FILE* pFILE;
    if((pFILE = fopen(file_name, "w")) == NULL)
        ERROR_MSG("Cannot create input image\n");

    fwrite("P2\n", sizeof(char), 3, pFILE);

    char tmp[16];
    sprintf(tmp, "%i %i\n", W, H);
    fwrite(&tmp, sizeof(char), strlen(tmp), pFILE);

    fwrite("255\n", sizeof(char), 4 , pFILE);

    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            sprintf(tmp, "%i ", J[j][i]);
            fwrite(&tmp, sizeof(char), strlen(tmp), pFILE);
        }
        fwrite("\n", sizeof(char), 1, pFILE);
    }

    fclose(pFILE);
}


void threads_handler() {
    pthread_t thread_id[Threads];
    struct thread_data tmp[Threads];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (int i = 0; i < Threads; ++i) {
        tmp[i].thread_num = i;
        if (pthread_create(&thread_id[i], &attr, thread_function, (void *) &tmp[i]) == -1)
            ERROR_MSG("Couldn't create a thread\n");
    }

    pthread_attr_destroy(&attr);
    for (int i = 0; i < Threads; ++i) {
        pthread_join(thread_id[i], NULL);
    }


}

int min(int a, int b){
    if (a > b)
        return b;
    else
        return a;
}

double filter_fun(int x, int y){

    double s = 0;
    for (int i = 1; i <= C; ++i) {
        int a = min((int) round(fmax(1, x - ceil((double) C / 2) + i)), W);

        for (int j = 1; j <= C; ++j) {

            int b = min((int) round(fmax(1, y - ceil((double) C / 2) + j)), H);
            s += I[a-1][b-1] * K[i-1][j-1];
        }
    }

    return s;
}


void *thread_function(void* arg){
    struct thread_data *my_data;
    my_data = (struct thread_data *) arg;
    int i;
    int j;
    struct timespec tstart, tend;

    clock_gettime(CLOCK_REALTIME, &tstart);

    int strapWidth = ceil((double)W / Threads);
    int start = (my_data->thread_num) * strapWidth;
    int end = ((start + strapWidth) < W) ? (start + strapWidth) : W;


    for (i = start; i < end; ++i) {
        for (j = 0; j < H; ++j) {
            J[i][j] = abs(round(filter_fun(i+1, j+1)));
        }
    }
    clock_gettime(CLOCK_REALTIME, &tend);

    long long t;
    if(C == 64)
        t = tend.tv_sec - tstart.tv_sec;
    else
        t = (tend.tv_nsec - tstart.tv_nsec);

    printf("Watek: %i - %lli\n", my_data->thread_num, t);

    return (void *) 0;
}


void prog_init(int argc, char* argv[]){
    if (argc < 5)
        ERROR_MSG("Too few arguments\n");

    if ((Threads = atoi(argv[1])) < 1)
        ERROR_MSG("Number of threads must be more than 0\n");

    char* input_file_name = argv[2];
    FILE* input_file;           /* handlers for files */
    FILE* filter_file;

    if((input_file = fopen(input_file_name, "r")) == NULL)
        ERROR_MSG("Cannot open input image file\n");
    parse_input_image(input_file);

    if((filter_file = fopen(argv[3], "r")) == NULL)
        ERROR_MSG("Cannot open filter file\n");
    parse_filter_file(filter_file);
}


void parse_filter_file(FILE *pFILE) {
    char* tmp = NULL;
    size_t len = 0;
    ssize_t read;
    char* token;

    getline(&tmp, &len, pFILE);
    DELETE_ENDLINE(tmp);
    C = atoi(tmp);
    if(C < 2 || C > 65)
        ERROR_MSG("Wrong pixel filter C\n");

    K = (double**) calloc(C, sizeof(double*));
    for (int k = 0; k < C; ++k) {
        K[k] = calloc(C, sizeof(double));
    }

    int i = 0, j;
    double sum = 0;  // at the end must be 1
    while((read = getline(&tmp, &len, pFILE)) != -1){
        j = 0;
        token = strtok(tmp, " ");

        while (token != NULL){
            double tmpF = atof(token);
            K[i][j] = tmpF;
            j++;
            sum += tmpF;
            token = strtok(NULL, " ");
        }
        i++;
    }

    fclose(pFILE);

    //if (fabs(1.0 - sum) > EPSILION)
    //    ERROR_MSG("Sum of filter matrix is not 1\n");
}


void parse_input_image(FILE *pFILE) {
    char* tmp = NULL;
    size_t len = 0;
    ssize_t read;
    char* token;

    getline(&tmp, &len, pFILE);
    tmp[strlen(tmp)-1] = '\0';
    if (strcmp(tmp, "P2") != 0)
        ERROR_MSG("Input file in wrong formating not P2\n");

    getline(&tmp, &len, pFILE);
    tmp[strlen(tmp)-1] = '\0';
    // getting out the W and H
    token = strtok(tmp, " ");
    W = atoi(token);
    token = strtok(NULL, " ");
    H = atoi(token);

    // allocating the I and J matrix
    I = (int**)malloc(W* sizeof(int*));
    for(int k = 0; k < W; k++) {
        I[k] = malloc(H* sizeof(int));
    }

    J = (int**)malloc(W* sizeof(int*));
    for (int k = 0; k < W; ++k) {
        J[k] = malloc(H* sizeof(int*));
    }

    // checking if the pixel is good
    getline(&tmp, &len, pFILE);
    DELETE_ENDLINE(tmp);
    if(atoi(tmp) != MAX_PIXEL)
        ERROR_MSG("Wrong pixel \n");

    // starting filling the I matrix
    int i = 0, j;
    while((read = getline(&tmp, &len, pFILE)) != -1){
        token = strtok(tmp, " ");
        j = 0;

        while(token != NULL){
            I[j][i] = atoi(token);
            j++;
            token = strtok(NULL, " ");
        }
        i++;
    }

    fclose(pFILE);
}