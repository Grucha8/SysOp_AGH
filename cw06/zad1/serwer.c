#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "constanst.h"

#define ERROR_MSG(format, ...) { printf(format, ##__VA_ARGS__); exit(-1);}

int clients[MAX_CLIENTS][2];            //[_][0] - pid; [_][1] - queueID
int numberOfClients = 0;

int queueID;
struct msgbuf message;

void mirror_handle();
void init_handle();
void calc_handle();
void time_handle();
void end_handle();
int getClientQueueID();
void stop_handle();

void close_queue(){
    msgctl(queueID, IPC_RMID, NULL);
    printf("Server shutdown\n");
}

void int_handler(int signum){
    close_queue();
    exit(1);
}

int main(){
    signal(SIGINT, int_handler);

    atexit(close_queue);

    /* setting the clients array */
    /* if [_][0] == -1 -> a free space for client */
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i][0] = -1;
    }

    char* pathname;

    if ((pathname = getenv("HOME")) == NULL)
        ERROR_MSG("SERVER: couldn't get the pathname\n");

    /* creating the main queue */
    key_t key;
    if ((key = ftok(pathname, SERWER_ID)) < 0)
        ERROR_MSG("SERVER: couldn't generate server queue key\n");

    if ((queueID = msgget(key, IPC_CREAT | IPC_EXCL | 0666)) < 0)
        ERROR_MSG("SERVER: couldn't create server queue\n");

    /* server main loop */
    while (1){

        if (msgrcv(queueID, &message, sizeof(struct msgbuf) - sizeof(long), 0, 0) < 0)
            ERROR_MSG("SERVER: couldn't receive message\n");

        /* checking the message */
       // if (&message == NULL) continue;      //no message

        switch (message.mtype){
            case INIT:
                init_handle();
                break;
            case MIRROR:
                mirror_handle();
                break;
            case CALC:
                calc_handle();
                break;
            case TIME:
                time_handle();
                break;
            case STOP:
                stop_handle();
                break;
            case END:
                end_handle();
                break;
            default:
                break;
        }
    }

    return 0;

}

void stop_handle() {
    int clientQueueID = getClientQueueID();

    if (clientQueueID == -1) return;

    message.pid = getpid();
    strcpy(message.msg, "ok");

    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[i][1] == clientQueueID) {
            clients[i][0] = -1;
            break;
        }
    }

    printf("SERVER: client-%d of id: %d shutdown complete\n", message.pid, i);
}

void end_handle() {
    if (msgctl(queueID, IPC_RMID, NULL) < 0)
        ERROR_MSG("SERVER: error with deleting queue\n")
    exit(0);
}

void time_handle() {
    int clientQueueID = getClientQueueID();
    if(clientQueueID == -1) return;

    message.pid = getpid();

    /* seeting the message */
    time_t  t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(message.msg, "%d-%d-%d %d",
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour);

    if(msgsnd(clientQueueID, &message, sizeof(struct msgbuf) - sizeof(long), 0) < 0)
        ERROR_MSG("SERVER: couldn't send message\n");

}

void calc_handle() {
    int clientQueueID = getClientQueueID();
    if(clientQueueID == -1) return;

    message.pid = getpid();

    if (strlen(message.msg) > 3)
    {
        printf("SERVER: too big numbers\n");
        return;
    }

    operation op = ERR;

    if (message.msg[1] == '+') { op = ADD; }
    else if (message.msg[1] == '-') { op = SUB; }
    else if (message.msg[1] == '*') { op = MUL; }
    else if (message.msg[1] == '/') { op = DIV; }


    int a, b;

    a = message.msg[0] - '0';
    b = message.msg[2] - '0';

    float result = -1;

    switch (op) {
        case ADD:
            result = a + b;
            break;
        case SUB:
            result = a - b;
            break;
        case MUL:
            result = a * b;
            break;
        case DIV:
            result = (float) a / b;
            break;
        case ERR:
            printf("SERVER: wrong operation\n");
            return;
    }

    printf("%f\n", result);

    sprintf(message.msg, "%f", result);

    if(msgsnd(clientQueueID, &message, sizeof(struct msgbuf) - sizeof(long), 0) < 0)
        ERROR_MSG("SERVER: couldn't send message\n");

}

void mirror_handle() {
    int clientQueueID = getClientQueueID();

    if (clientQueueID == -1) return;

    /* creating message */
    message.pid = getpid();

    int len = strlen(message.msg);

    char buf[len];

    for (int i = 0; i < len; ++i) {
        buf[i] = message.msg[len - i - 1];
    }

    strcpy(message.msg, buf);

    if (msgsnd(clientQueueID, &message, sizeof(struct msgbuf) - sizeof(long), 0) < 0)
        ERROR_MSG("SERVER: couldn't send message");
}



void init_handle() {
    key_t clientKey;

    clientKey = atoi(message.msg);

    /* opening the client priavte queue */
    int clientQueueID;
    if ((clientQueueID = msgget(clientKey, 0)) < 0)
        ERROR_MSG("SERVER: couldn't connect to client private queue");

    if(numberOfClients > MAX_CLIENTS){
        printf("SERVER: too many clients");
    }
    else{
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(clients[i][0] == -1){
                clients[i][0] = message.pid;
                clients[i][1] = clientQueueID;
                sprintf(message.msg, "%d", i);
                numberOfClients++;
                break;
            }
        }
    }

    if (msgsnd(clientQueueID, &message, sizeof(struct msgbuf) - sizeof(long), 0) < 0)
        ERROR_MSG("SERVER: couldn't send reply message to client");
}

/* helper functions */

int getClientQueueID() {

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i][0] == message.pid)
            return clients[i][1];
    }

    printf("SERVER: couldn't find client");
    return -1;
}