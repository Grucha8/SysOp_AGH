#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

void err_sys(char* a){
    printf("%s\n", a);
    exit(1);
}

void obslugaINT(int signum){
    killpg(0, SIGINT);
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

    signal(SIGINT, obslugaINT);

    char* namedPipe = argv[1];

    numberOfSlaves = atoi(argv[2]);

    N = argv[3];

    //printf("%s - %i - %c\n", namedPipe, numberOfSlaves, N);

    slaves = calloc((size_t) atoi(N), sizeof(pid_t));

    if((master = fork()) < 0){
        err_sys("master fork error");
    }
    else if(master == 0){
        execlp("./master", "master", namedPipe, NULL);
        exit(0);
    }

    for (int i = 0; i < numberOfSlaves; ++i) {
        if((slaves[i] = fork()) < 0){
            err_sys("slave fork error");
        }
        else if(slaves[i] == 0){
            execlp("./slave", "slave", namedPipe, N, NULL);
            exit(0);
        }
    }


    //TODO waitng
    for (int i = 0; i < numberOfSlaves; ++i) {
        waitpid(slaves[i], NULL, WUNTRACED);
        printf("%d'th slave exited\n", i);
    }

    kill(master, SIGINT);
    killpg(0, SIGINT);
    waitpid(master, NULL, 0);

    return 0;
}