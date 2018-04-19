#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

void err_sys(char* a){
    printf("%s\n", a);
    exit(1);
}

pid_t master;
pid_t* slaves;
int numberOfSlaves; //liczba slave'ow
char* N;

//argv[1] - potok,
//argv[2] - liczba slave'ow,
//argv[3] - N
int main(int argc, char* argv[]){
    if(argc != 4){
        err_sys("za malo argumentow");
    }

    char* namedPipe = argv[1];

    numberOfSlaves = atoi(argv[2]);

    N = argv[3];

    slaves = calloc((size_t) N, sizeof(pid_t));

    for (int i = 0; i < numberOfSlaves; ++i) {
        if((slaves[i] = fork()) < 0){
            err_sys("slave fork error");
        }
        else if(slaves[i] == 0){
            execlp("./slave", "slave", namedPipe, N, NULL);
            exit(0);
        }
    }

    if((master = fork()) < 0){
        err_sys("master fork error");
    }
    else if(master == 0){
        execlp("./master", "master", namedPipe, NULL);
        exit(0);
    }



    //TODO waitng

    kill(master, SIGINT);
    waitpid(master, NULL, 0);

    return 0;
}