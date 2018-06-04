#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <mqueue.h>

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


mqd_t queueID = -1;
mqd_t privateQueueID;
char privateKey[20];
struct msgbuf message;

void int_handler(int signum){
    //close_queue();
    exit(1);
}

void close_queue(){
    /* sending message to server to close my queue */
    if (queueID != -1){
        message.pid = getpid();
        message.mtype = STOP;

        mq_send(queueID, (char*) &message, sizeof(struct msgbuf), 2);
    }

    /* closing server queue */
    mq_close(queueID);

    /* closing and deleting my queue */
    mq_close(privateQueueID);
    mq_unlink(privateKey);

    printf("\nClient shutdown\n");
}

int main(){
    atexit(close_queue);

    signal(SIGINT, int_handler);

    /* connecting with the server queue */
    if ((queueID = mq_open(PATH, O_WRONLY)) < 0)
    ERROR_MSG("Nie udalo sie otworzyc kolejki");

    /* tworzenie prywatnej kolejki */
    sprintf(privateKey, "/%d", getpid());

    struct mq_attr attr;
    attr.mq_msgsize = sizeof(struct msgbuf);
    attr.mq_maxmsg = MAX_MSG;

    if((privateQueueID = mq_open(privateKey, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr)) < 0)
    ERROR_MSG("Nie udalo sie stworzyc prywatnej kolejki")

    /* ustawianie wiadomosci i jej wyslanie*/
    /* ta zmienna bedzie sluzyla nam do konca trwania programu */
    message.pid = getpid();
    message.mtype = INIT;
    //sprintf(message.msg, "%d", privateKey);

    if(mq_send(queueID, (char*) &message, sizeof(struct msgbuf), 10) < 0)
    ERROR_MSG("Nie udalo sie przeslac wiadomosci inicjujacej");

    /* odebranie identyfikatora od serwera */
    if(mq_receive(privateQueueID, (char*) &message, sizeof(struct msgbuf), NULL) < 0)
    ERROR_MSG("Nie udalo sie odebrac ID klienta od serwera");

    int myID = atoi(message.msg);
    printf("My id is: %d\n", myID);

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
        printf("Wrong command");

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
            printf("Wrong command");

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

    if (mq_send(queueID, (char*) &message, sizeof(struct msgbuf), 10) < 0)
    ERROR_MSG("Couldn't send message to server");
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
    if (mq_send(queueID, (char*) &message, sizeof(struct msgbuf), 10) < 0)
    ERROR_MSG("Couldn't send message to server");

    if (mq_receive(privateQueueID, (char*) &message, sizeof(struct msgbuf), NULL) < 0)
    ERROR_MSG("Couldn't recevie message from server");

}

