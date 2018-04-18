#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

//max 5 argumentow

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
    size_t      len = 100;
    ssize_t     nread;

    char**        argv;
    const char  s[] = " ";    //stala trzymajaca odzielenie
    char*       token;

    argv = (char**)calloc(6, sizeof(char*));
    for (int i = 0; i < 6; ++i) {
        argv[i] = calloc(20, sizeof(char));
    }

    //zaczynamy parsowanie z pliku
    while ((nread = getline(&line, &len, file)) != -1){
        token = strtok(line, s);

        for (int i = 0; i < 6; ++i) {
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

    }

}

/*
 * @brief: funkcja wykonujaca polecenie
 * @params: argv - tablica z poleceniem oraz atrybutami
 */
void exec_func(char **argv) {
    pid_t  pid;
    int    status;

    pid = fork();

    if( pid == 0 ){
        if ( execvp(argv[0], argv) == -1 ){
            exit(1);
        }
    }
    else if ( pid > 0 ){
        waitpid(pid, &status, WUNTRACED);

        //sprawdzanie czy potomek dobrze sie zakonczyl
        if( WIFEXITED(status) && WEXITSTATUS(status) != 0 ){
            printf("Blad w wykonywaniu polecenia: %s\n", argv[0]);
            exit(2);
        }
    }
    else{
        perror("Nie udalo sie uruchomic procesu");
        exit(1);
    }

}
