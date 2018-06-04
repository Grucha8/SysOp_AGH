
#ifndef AAA_HELPER_H
#define AAA_HELPER_H

#define BARBER_ID 888
#define MAX_CLIENTS 16

#define ERROR_MSG(format, ...) { printf(format, ##__VA_ARGS__); exit(-1);}

/* shared memory */
struct Barbershop* barbershop;

/* enum struct for barber status */
enum barber_status{
    SLEEPING = 0, WORKING = 1, WAITING = 2, AWEKING = 3
};

/* enum struct for client status */
enum client_status{
    NEW = 0, SHAVING = 1, AWAITING = 2
};


/* barbershop struct */
struct Barbershop{
    pid_t queue[MAX_CLIENTS];   /* the queue for storaging clients */
    pid_t current_client;
    int max_amount_of_clients;  /* the first argument of program */
    int waiting_clients;        /* number of waiting clients */
    enum barber_status status;  /* status of the barber */

};

#endif //AAA_HELPER_H
