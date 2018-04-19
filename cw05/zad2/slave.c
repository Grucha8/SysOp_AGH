#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 128

void err_sys(char* a){
    printf("%s\n", a);
    exit(1);
}

//[1] -namedPipe, [2]-N
int main(int argc, char* argv[]){
    if(argc != 3){
        err_sys("Slave: zla ilosc argumentow");
    }

    srand((unsigned int) getpid());

    char* namedPipe = argv[1];

    int N = atoi(argv[2]);

    FILE* pipeFile;

    FILE* date;

    char* dateBuffer = calloc((size_t) MAXLINE, sizeof(char));
    char buffer[MAXLINE];

    size_t sizeBuf;

    if((pipeFile = fopen(namedPipe, "a")) == NULL){
        err_sys("Slave: fopen error");
    }

    pid_t pid = getpid();

    printf("Slave PID: %d\n", pid);

    for (int i = 0; i < N; ++i) {

        if((date = popen("date", "r")) == NULL){
            err_sys("Slave: popen error");
        }  //uruchomienie date

        getline(&dateBuffer, &sizeBuf, date);

        //interpolacja stringa
        sprintf(buffer, "PID: %d - %s", pid, dateBuffer);

        fwrite(buffer, sizeof(char), MAXLINE, pipeFile);

        //spimy 2-5 sec
        sleep((unsigned) (rand()%(5-1) + 2));
    }

    free(dateBuffer);

    fclose(pipeFile);
    exit(0);

}

