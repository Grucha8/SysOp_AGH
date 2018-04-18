#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXPARAMETERS 8
#define MAXCOMMANDS 8
#define MAXCOMMANDSIZE 256

// 8 * 8 = 64

static void err_sys(char* message){
    printf("%s\n", message);
    exit(1);
}

void reading_from_file(FILE* file);
void exec_func(char **argv);

int main(int argc, char* argv[]){

    if (argc < 2){
        printf("Za mala ilosc argumentow\n");
        return 1;
    }

    char* file_name = argv[1];

    FILE* file;

    if ( (file = fopen(file_name, "r")) == NULL){
        perror("Nie udalo sie otworzyc pliku\n");
        return 1;
    }

    reading_from_file(file);

}

void reading_from_file(FILE* file){
    char*       line = NULL;
    size_t      len = 200;        // !!!
    ssize_t     nread;

    char**      argv;
    const char  s[] = "|";    //stala trzymajaca odzielenie
    char*       token;

    argv = (char**)calloc(MAXCOMMANDS, sizeof(char*));
    for (int i = 0; i < MAXCOMMANDS; ++i) {
        argv[i] = calloc(MAXCOMMANDSIZE, sizeof(char));
    }

    //zaczynamy parsowanie z pliku
    while ((nread = getline(&line, &len, file)) != -1){
        token = strtok(line, s);

        for (int i = 0; i < MAXCOMMANDS; ++i) {
            if( token == NULL ){
                argv[i] = NULL;
            }
            else {
                strcpy(argv[i], token);

                token = strtok(NULL, s);

                /*
                 * ogolnie to jest problem jak mamy
                 * linie kodu to wtedy ostatni argument
                 * konczy sie znakiem nowej lini wiec
                 * trzeba go usunac
                 */
                if (token == NULL){
                    char* tmp;
                    //wyciagamy wskaznik na ostatni pojawienie sie \n
                    if ((tmp = strrchr(argv[i], '\n')) == NULL)
                        continue;
                    tmp[0] = 0;
                }
            }
        }

        exec_func(argv);

        //uwalnianie pamieci
        for(int i = 0; i < MAXCOMMANDS; i++)
            free(argv[i]);
        free(argv);

    }
}

char** separatePipe(char *argv){
    char**      func;

    const char  s[] = " ";    //stala trzymajaca odzielenie
    char*       token;

    func = (char**)calloc(MAXPARAMETERS, sizeof(char*));
    for (int i = 0; i < MAXPARAMETERS; ++i) {
        func[i] = calloc(32, sizeof(char));
    }

    token = strtok(argv, s);

    for(int i = 0; i < MAXPARAMETERS; i++) {
        if(token == NULL){
            func[i] = NULL;
        }
        else {
            strcpy(func[i], token);

            token = strtok(NULL, s);
        }
    }


    //teraz mamy w func[] polecenie do odpalenia

    return func;


    for(int i = 0; i < MAXPARAMETERS; i++)
        free(func[i]);
    free(func);

}

pid_t prevPid;

/*
 * @brief: funkcja wykonujaca polecenie
 * @params: argv - tablica z poleceniem oraz atrybutami
 *
 */
void exec_func(char **argv) {
    pid_t  pid;
    char** func;

    int fd[MAXCOMMANDS][2];

    int i;
    for(i = 0; i < MAXCOMMANDS && argv[i] != NULL; i++){
        func = separatePipe(argv[i]);

        for(int j = 0; func[j] != NULL; j++)
            printf("%s ", func[j]);
        printf("\n==================\n");

        //odpalenie "rury"
        if(pipe(fd[i]) < 0){
            err_sys("pipe error");
        }

        if((pid = fork()) < 0)
            err_sys("fork error");
        else if(pid == 0){  //potomek

            if(i != 0){             //wszystkie programy oprocz pierwszego wczytuja
                if(fd[i-1][0] != STDIN_FILENO){
                    if(dup2(fd[i-1][0], STDIN_FILENO) != STDIN_FILENO)
                        err_sys("dup2 error in stdin");
                }
                close(fd[i-1][0]);
            }

            if(argv[i+1] != NULL){  //wszystkie programy oprocz ostatniego wpisuja
                if(fd[i][1] != STDOUT_FILENO){
                    if(dup2(fd[i][1], STDOUT_FILENO) != STDOUT_FILENO)
                        err_sys("dup2 error in stdout");
                }
                close(fd[i][1]);
            }

            if(execvp(func[0], func) < 0){
                printf("execvp error for %s\n", func[0]);
                exit(1);
            }

            exit(0);
        }

        prevPid = pid;
    }



    wait(NULL);
    exit(0);
}
