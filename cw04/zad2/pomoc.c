#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void usrHandler(int signum) {
    kill(getppid(), SIGRTMIN + (rand() % (SIGRTMAX - SIGRTMIN)));
}

int main() {
    srand((unsigned int) getpid());

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);

    unsigned int a = (rand() % 11);

    printf("PROCES: %d - spi: %d\n", getpid(), a);
    sleep(a);

    //wyslanie sygnalu o pozwolenie pracy do taty
    kill(getppid(), SIGUSR1);

    sigsuspend(&mask);

    //zwracanie ile sekund spal
    return a;
}