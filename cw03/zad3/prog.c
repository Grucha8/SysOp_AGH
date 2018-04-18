//
// Created by tkarkocha on 27.03.18.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

//max 5 argumentow
static rlim_t time_limit;
static rlim_t space_limit;

void reading_from_file(FILE* file);
void exec_func(char **argv);
void set_limits();
void print_usage(struct rusage usage);

int main(int argc, char* argv[]){

    if (argc < 4){
        printf("Za mala ilosc argumentow\n");
        return 1;
    }

    char* file_name = argv[1];

    time_limit = atoi(argv[2]);
    space_limit = atoi(argv[3]) * 1000 * 1000; //zmienianie na megabajty


    FILE* file;

    if ( (file = fopen(file_name, "r")) == NULL){
        perror("Nie udalo sie otworzyc pliku\n");
        return 1;
    }


    reading_from_file(file);

    fclose(file);


    return 0;

}

/*
 * @brief:  funkcja czytajac linijke z
 *          pliku zawierajacego zadania wsadowe
 *          oraz rozpoczynajaca funkcje
 *          do uruchamiania tych polecen
 *
 * @params: file - uchwyt do pliku zawierajacego
 *          zadania wsadowe
 */
void reading_from_file(FILE* file){

    char*       line = NULL;
    size_t      len = 200;
    ssize_t     nread;

    char**      argv;
    const char  s[] = " ";    //stala trzymajaca odzielenie
    char*       token;

    //zaczynamy parsowanie z pliku
    while ((nread = getline(&line, &len, file)) != -1){

        argv = (char**)calloc(6, sizeof(char*));
        for (int i = 0; i < 6; ++i) {
            argv[i] = calloc(40, sizeof(char));
        }

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

       /* for (int j = 0; j < 6; ++j) {
            printf("%s ", argv[j]);
        }
        printf("\n");
        */
        exec_func(argv);

        free(argv);

    }

}

/*
 * @brief:  funkcja wykonujaca polecenie
 *
 * @params: argv - tablica z poleceniem oraz atrybutami
 */
void exec_func(char **argv) {
    static struct rusage    usage;
    pid_t                   pid;
    int                     status;

    pid = fork();

    if( pid == 0 ){
        //ustaw limity
        set_limits();


        if ( execvp(argv[0], argv) == -1 ){
            exit(1);
        }
    }
    else if ( pid > 0 ){
        //waitpid(pid, &status, WUNTRACED);
        wait3(&status, WUNTRACED, &usage);

        //sprawdzanie czy potomek dobrze sie zakonczyl
        if( WIFEXITED(status) && WEXITSTATUS(status) != 0 ){
            printf("Blad w wykonywaniu polecenia: %s\n", argv[0]);
            exit(2);
        }
        else { //jesli potomek sie spokojnie zakonczyl wypisz jego zuzycie
            printf("========\n");

            if(status == SIGKILL){
                printf("twarde ograniczenie zostalo przekroczone\n");
            }
            else if (status == SIGSEGV){
                printf("blad segmentacji - zuzyto za duzo pamieci\n");
            }


            //wypisanie polecenia
            for (int i = 0; i < 6; ++i) {
                if(argv[i] == NULL) break;
                printf("%s ", argv[i]);
            }

            printf("\n");
            print_usage(usage);
            printf("\n");
        }
    }
    else{
        perror("Nie udalo sie uruchomic procesu");
        exit(1);
    }

}


/*
 * @brief:  funkcja ustawiajaca limity dla
 *          czasu dostepu do procesora
 *          i rozmiaru wirtualnej pamieci
 *          dla procesu
 */
void set_limits() {
    struct rlimit   rlptr = {time_limit, time_limit};

    if ( setrlimit(RLIMIT_CPU, &rlptr) < 0 ){
        printf("cpu limit\n");
        perror("blad w ustawianiu ograniczenia na czas dostepu do procesora\n");
        exit(1);
    }

    rlptr.rlim_cur = space_limit;
    rlptr.rlim_max = space_limit;

    if( setrlimit(RLIMIT_AS, &rlptr) < 0 ){
        printf("mem limit\n");
        perror("blad w ustawianiu ograniczenia na pamiec\n");
        exit(1);
    }

}


/*
 * @brief:  krotka funkcja do drukowania
 *          zuzycia procesu
 */
void print_usage(struct rusage usage){
    printf("Zuzycie user CPU: %ld mikrosekund\nZuzycie sys CPU: %ld mikrosekund\n", usage.ru_utime.tv_usec, usage.ru_stime.tv_usec);
    printf("Zuzycie mem: %ld\n", usage.ru_maxrss);
}