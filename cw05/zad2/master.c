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

    //bufer sluzacy do czytania z pipe
    char* buffer = calloc((size_t) MAXLINE, sizeof(char));

    FILE* pipeFile;

    if(mkfifo(namedPipe, S_IWUSR | S_IRUSR) < 0){
        err_sys("Master: mkfifo error");
    }

    //otwieramy pipe
    if((pipeFile = fopen(namedPipe, "r")) == NULL){
        err_sys("Master: fopen error");
    }

    size_t sizeBuf = MAXLINE;

    while(getline(&buffer, &sizeBuf, pipeFile) != 0){
        printf("%s", buffer);
    }

    fclose(pipeFile);

    exit(0);

}

