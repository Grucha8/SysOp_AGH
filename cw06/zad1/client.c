#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "constanst.h"

#define ERROR_MSG(format, ...) { printf(format, ##__VA_ARGS__); exit(-1);}

#define CMD_PATH "./cmd.txt"
void reading_from_file();
void reading_from_console();
void mirror_handler();
void calc_handler();
void time_handler();
void end_handler();
void communicate();

int queueID = -1;
int privateQueueID;
int myID;
struct msgbuf message;


void close_queue(){
    if (queueID != -1){
        message.pid = getpid();
        message.mtype = STOP;

        msgsnd(queueID, &message, sizeof(struct msgbuf) - sizeof(long), 0);
    }

    msgctl(privateQueueID, IPC_RMID, NULL);
    printf("\nClient shutdown\n");
}

void int_handler(int signum){
    close_queue();
    exit(1);
}

int main(){
    atexit(close_queue);

    signal(SIGINT, int_handler);

    char* pathname;

    if((pathname = getenv("HOME")) == NULL)
        ERROR_MSG("Blad przy zdobyciu sciezki katalogu\n");

    /* otworzenie kolejki serwera */
    key_t key;
    if ((key = ftok(pathname, SERWER_ID)) < 0)
        ERROR_MSG("Nie udalo sie ustawic klucza\n");

    if ((queueID = msgget(key, 0)) < 0)
        ERROR_MSG("Nie udalo sie otworzyc kolejki\n");

    /* tworzenie prywatnej kolejki */
    key_t privateKey;
    if ((privateKey = ftok(pathname, getpid())) < 0)
        ERROR_MSG("Nie udalo sie ustawic prywatnego klucza\n");

    if((privateQueueID = msgget(privateKey, IPC_CREAT | IPC_EXCL | 0666)) < 0)
        ERROR_MSG("Nie udalo sie stworzyc prywatnej kolejki\n")

    /* ustawianie wiadomosci i jej wyslanie*/
    /* ta zmienna bedzie sluzyla nam do konca trwania programu */
    message.pid = getpid();
    message.mtype = INIT;
    sprintf(message.msg, "%d", privateKey);

    if(msgsnd(queueID, &message, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT) < 0)
        ERROR_MSG("Nie udalo sie przeslac wiadomosci inicjujacej\n");

    /* odebranie identyfikatora od serwera */
    if(msgrcv(privateQueueID, &message, sizeof(struct msgbuf) - sizeof(long), 0, 0) < 0)
        ERROR_MSG("Nie udalo sie odebrac ID klienta od serwera\n");

    myID = atoi(message.msg);

    /* mozemy rozpoczac komunikacje z serwerem */
    char cmd[16];

    printf("Podaj polecenie(plik lub cons): ");
    scanf("%s", cmd);
    /*
     * cmd == "plik" - czytamy z pliku
     * cmd == "cons" - czytamy z konsoli
     */

    if (strcmp(cmd, "plik") == 0)
        reading_from_file();
    else if (strcmp(cmd, "cons") == 0)
        reading_from_console();
    else
        printf("Wrong command\n");

}

void reading_from_console() {
    char buf[16];

    while (1){
        printf("Write command: ");
        scanf("%s", buf);

        if(strcmp(buf, "MIRROR") == 0)
            mirror_handler();
        else if(strcmp(buf, "CALC") == 0)
            calc_handler();
        else if(strcmp(buf, "TIME") == 0)
            time_handler();
        else if(strcmp(buf, "END") == 0) {
            end_handler();
            exit(0);
        }
        else
            printf("Wrong command\n");

    }
}

void reading_from_file() {
    FILE* file;
    if((file = fopen(CMD_PATH, "r")) == NULL)
    ERROR_MSG("couldn't open file\n");

    size_t cos = 100;
    char* buf = NULL;

    while (getline(&buf, &cos, file) != -1){
        buf[strcspn(buf, "\n")] = 0;

        printf("%s\n", buf);

        if(strcmp(buf, "MIRROR") == 0)
            mirror_handler();
        else if(strcmp(buf, "CALC") == 0)
            calc_handler();
        else if(strcmp(buf, "TIME") == 0)
            time_handler();
        else if(strcmp(buf, "END") == 0) {
            end_handler();
            exit(0);
        }
        else
            printf("Wrong command\n");

        strcpy(buf, "");
    }
}

void end_handler() {
    message.mtype = END;
    message.pid = getpid();

    if (msgsnd(queueID, &message, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT) < 0)
        ERROR_MSG("Couldn't send message to server\n");
}

void time_handler() {
    message.mtype = TIME;
    message.pid = getpid();


    communicate();

    printf("%s\n", message.msg);
}

void calc_handler() {
    message.mtype = CALC;
    message.pid = getpid();

    printf("Please write the expression: ");
    scanf("%s", message.msg);

    communicate();

    printf("%s\n", message.msg);
}

void mirror_handler() {
    /* inicjacja wiadomosci */
    message.mtype = MIRROR;
    message.pid = getpid();

    printf("Word to mirror it: ");
    scanf("%s", message.msg);

    communicate();

    printf("%s \n", message.msg);
}

void communicate() {
    if (msgsnd(queueID, &message, sizeof(struct msgbuf) - sizeof(long), IPC_NOWAIT) < 0)
    ERROR_MSG("Couldn't send message to server\n");

    if (msgrcv(privateQueueID, &message, sizeof(struct msgbuf) - sizeof(long), 0, 0) < 0)
    ERROR_MSG("Couldn't recevie message from server\n");

}

