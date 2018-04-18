#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <bits/siginfo.h>

static int N;
static int K;
static int waitingChilds;
static int runningChilds;
pid_t *pidArray;
pid_t *pidAwaiting;

void obslugaINT(int signum, siginfo_t *info, void *context);
void obslugaUSR1(int signum, siginfo_t *info, void *context);
void obslugaCHLD(int signum, siginfo_t *info, void *context);
void obslugaRT(int signum, siginfo_t *info, void *context);


int main(int argc, char* argv[]){
    if(argc < 3){
        printf("Za malo argumentow");
        exit(1);
    }


    N = atoi(argv[1]);
    K = atoi(argv[2]);

    //tablica trzymajaca pid dzieci
    pidArray = calloc((size_t) N, sizeof(pid_t));
    pidAwaiting = calloc((size_t) N, sizeof(pid_t));

    if(N < K){
        printf("K nie moze byc wieksze od N\n");
        exit(1);
    }

    waitingChilds = 0;
    runningChilds = 0;

    //inicjaca do sigaction
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO; //odpowiedzialne za zwracanie wartosci przez sygnal

    //inicjacja wszystkich funkcji do obslugi syganlow
    act.sa_sigaction = obslugaINT;
    sigaction(SIGINT, &act, NULL);

    act.sa_sigaction = obslugaUSR1;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_sigaction = obslugaCHLD;
    sigaction(SIGCHLD, &act, NULL);

    for (int i = SIGRTMIN; i <= SIGRTMAX; i++){
        act.sa_sigaction = obslugaRT;
        sigaction(i, &act, NULL);
    }

    for(int i = 0; i < N; i++){
        pid_t pid = fork();

        if(pid == 0){
            execl("./pomoc", "./pomoc", NULL);
            break;
        } else {
            pidArray[runningChilds] = pid;
            runningChilds++;
        }
    }

    while( 1 ){
        wait(NULL);
    }


}


void obslugaINT(int signum, siginfo_t *info, void *context) {
    printf("\nDostanie SIGINT\n");

    for (int i = 0; i < N; i++){
        if (pidArray[i] != -1){
            kill(pidArray[i], SIGKILL);
            waitpid(pidArray[i], NULL, 0);
        }
    }

    exit(0);
}

void obslugaUSR1(int signum, siginfo_t *info, void *context){
    pid_t tmpPid = info->si_pid;

    printf("SIGUSR1 z PID: %d\n", tmpPid);

    int flag = 0;

    //sprawdzanie czy potomek
    for (int i = 0; i < N; i++){
        if(pidArray[i] == tmpPid) flag = 1;
    }

    if(!flag){
        printf("To nie dziecko PID: %d\n", tmpPid);
        return;
    }


    if( waitingChilds >= K ){
        printf("Wysylanie SIGUSR1 do PID: %d\n", tmpPid);
        kill(tmpPid, SIGUSR1);
        waitpid(tmpPid, NULL, 0);
    }
    else {
        pidAwaiting[waitingChilds] = tmpPid;
        waitingChilds++;

        if(waitingChilds >= K){
            for (int i = 0; i < K; i++) {
                printf("Wysylanie SIGUSR1 do PID: %d\n", pidAwaiting[i]);
                kill(pidAwaiting[i], SIGUSR1);
                waitpid(pidAwaiting[i], NULL, 0);
            }
        }
    }
}

void obslugaCHLD(int signum, siginfo_t *info, void *context){
    if(info->si_code == CLD_EXITED) {
        printf("Dziecko %d zostalo zabite, exit status: %d\n", info->si_pid, info->si_status);
    }
    else {
        printf("Dziecko %d zostalo zabite przez sygnal: %d\n", info->si_pid, info->si_status);
    }
    runningChilds--;

    if(runningChilds == 0){
        printf("Brak dziecki, koncze prace...\n");
        exit(0);
    }

    for(int i = 0; i < N; i++){
        if(pidArray[i] == info->si_status){
            pidArray[i] = -1;
            break;
        }
    }
}

void obslugaRT(int signum, siginfo_t *info, void *context){
    int sig = signum - SIGRTMIN;
    printf("Sygnal SIGRT: SIGMIN+%i - PID: %d\n", sig, info->si_pid);
}
