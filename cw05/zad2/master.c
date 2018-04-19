#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLINE 128


void err_sys(char* a){
    printf("%s\n", a);
    exit(1);
}

int main(int argc, char* argv[]){
    if(argc != 2){
        err_sys("Master: zla ilosc argumentow");
    }

    char* namedPipe = argv[1];

    printf("KURWA%i\n", 1);
    //bufer sluzacy do czytania z pipe
    char* buffer = calloc((size_t) MAXLINE, sizeof(char));

    FILE* pipeFile;

    printf("KURWA%i\n", 2);
    if(mkfifo(namedPipe, S_IWUSR | S_IRUSR) < 0){
        err_sys("Master: mkfifo error");
    }

    printf("KURWA%i\n", 3);
    //otwieramy pipe
    if((pipeFile = fopen(namedPipe, "r")) == NULL){
        err_sys("Master: fopen error");
    }

    size_t sizeBuf;
    printf("KURWA%i\n", 4);

    while(getline(&buffer, &sizeBuf, pipeFile)){
        printf("%s", buffer);
    }

    fclose(pipeFile);

    exit(0);

}

