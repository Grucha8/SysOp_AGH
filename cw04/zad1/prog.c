#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

static int pid = -1;

static void obslugaSTOP(int signum);
static void obslugaINT(int signum);

int main(void) {
    //przygotowanie i ustawienie przechwytu sygnalu SIGINT
    struct sigaction act;
    act.sa_handler = obslugaINT;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    //ustawienie przechwytu sygnalu SIGTSTP
    signal(SIGTSTP, obslugaSTOP);

    while(1){
        pid = fork();

        if (pid > 0){
            wait(NULL);
            pause();
        }
        else if (pid == 0){
            if(execlp("./skrypt.sh", "skrypt.sh", NULL) == -1)
                exit(1);
        }
        }


    return 0;
}


/*
 * @brief:  funkcja obslugujaca sygnal SIGTSTP
 *
 * @params: signum - numer sygnalu
 */
void obslugaSTOP(int signum){

    if(kill(pid, SIGKILL) == -1){

    }
    else{
        printf("\nOczekuje na CTRL + Z - kontynuacja albo CTRL + C - zakonczenie programu\n");
    }


}

void obslugaINT(int signum){
    printf("Odebrana sygnal SIGINT, koncze prace...\n");

    kill(pid, SIGINT);
}
