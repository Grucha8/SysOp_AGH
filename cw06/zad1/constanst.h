#ifndef AAA_CONSTANST_H
#define AAA_CONSTANST_H

#define MAX_CLIENTS 6
#define MSG_SIZE 32
#define SERWER_ID 888

enum mtype {
    INIT = 1, MIRROR = 2, CALC = 3,
    TIME = 4, END = 5, STOP = 6
};

typedef enum operation{
    ERR = 1, ADD = 2, MUL = 3,
    SUB = 4, DIV = 5
} operation;

struct msgbuf {
    long mtype;
    pid_t pid;
    char msg[MSG_SIZE];
};

#endif
