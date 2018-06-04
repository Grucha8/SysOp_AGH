#define _GNU_SOURCE

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>

#include "constanst.h"

#define ERROR_MSG(format, ...) { printf(format, ##__VA_ARGS__); exit(-1);}

mqd_t clients[MAX_CLIENTS];            //[_][0] - pid; [_][1] - queueID
int clientsPid[MAX_CLIENTS];
int numberOfClients = 0;

mqd_t queueID;
struct msgbuf message;

void mirror_handle();
void init_handle();
void calc_handle();
void time_handle();
void end_handle();
mqd_t getClientQueueID();
void stop_handle();


void close_queue(){
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if(clientsPid[i] != -1){
            if (mq_close(clients[i]) < 0)
                ERROR_MSG("SERVER: couldn't close client queue %d\n", i);
        }
    }

    mq_close(queueID);
    mq_unlink(PATH);
    printf("Server shutdown\n");
}

void int_handler(int signum){
    //close_queue();
    exit(1);
}

int main(){
    signal(SIGINT, int_handler);

    atexit(close_queue);

    /* setting the clients array */
    /* if [_][0] == -1 -> a free space for client */
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clientsPid[i] = -1;
    }

    /* creating struct mq_attr */
    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = sizeof(struct msgbuf);

    /* creating the main queue */
    if ((queueID = mq_open(PATH, O_CREAT | O_RDONLY | O_EXCL, 0666, &attr)) < 0)
        ERROR_MSG("SERVER: couldn't create server queue");

    /* server main loop */
    while (1){
        if (mq_receive(queueID, (char *) &message, sizeof(struct msgbuf), NULL) < 0)
            ERROR_MSG("SERVER: couldn't receive message");

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

//handlers//

void stop_handle() {
    mqd_t clientQueueID = getClientQueueID();

    if (clientQueueID == -1) return;

    message.pid = getpid();
    strcpy(message.msg, "ok");

    int i;

    for (i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[i] == clientQueueID) {
            clientsPid[i] = -1;
            mq_close(clientQueueID);
            break;
        }
    }

    printf("SERVER: client-%d of id: %d shutdown complete\n", message.pid, i);

}

void end_handle() {
    if (mq_close(queueID) < 0)
        ERROR_MSG("SERVER: error with deleting queue\n")
}

void time_handle() {
    mqd_t clientQueueID = getClientQueueID();
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

    if(mq_send(clientQueueID, (char*) &message, sizeof(struct msgbuf), 1) < 0)
    ERROR_MSG("SERVER: couldn't send message\n");

}

void calc_handle() {
    mqd_t clientQueueID = getClientQueueID();
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


    printf("\n%f\n", result);

    sprintf(message.msg, "%f", result);

    if(mq_send(clientQueueID, (char*) &message, sizeof(struct msgbuf), 1) < 0)
        ERROR_MSG("SERVER: couldn't send message\n");

}

void mirror_handle() {
    mqd_t clientQueueID = getClientQueueID();

    if (clientQueueID == -1) return;

    /* creating message */
    message.pid = getpid();

    int len = strlen(message.msg);

    char buf[len];

    for (int i = 0; i < len; ++i) {
        buf[i] = message.msg[len - i - 1];
    }

    strcpy(message.msg, buf);

    if (mq_send(clientQueueID, (char*) &message, sizeof(struct msgbuf), 1) < 0)
    ERROR_MSG("SERVER: couldn't send message");
}



void init_handle() {
    /* opening the client priavte queue */
    char clientKey[20];
    sprintf(clientKey, "/%d", message.pid);

    mqd_t clientQueueID;
    if ((clientQueueID = mq_open(clientKey, O_WRONLY)) < 0)
    ERROR_MSG("SERVER: couldn't connect to client private queue");

    if(numberOfClients > MAX_CLIENTS){
        printf("SERVER: too many clients");
    }
    else{
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(clientsPid[i] == -1){
                clientsPid[i] = message.pid;
                clients[i] = clientQueueID;
                sprintf(message.msg, "%d", i);
                numberOfClients++;
                break;
            }
        }
    }

    if (mq_send(clientQueueID, (char*) &message, sizeof(struct msgbuf), 1) < 0)
    ERROR_MSG("SERVER: couldn't send reply message to client");
}

/* helper functions */

int getClientQueueID() {

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clientsPid[i] == message.pid)
            return clients[i];
    }

    printf("SERVER: couldn't find client");
    return -1;
}