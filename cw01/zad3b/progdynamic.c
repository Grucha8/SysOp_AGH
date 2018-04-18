//
// Created by tkarkocha on 12.03.18.
//

#include <stdio.h>
#include <sys/times.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dlfcn.h>
void *handle;   //uchwyt do biblioteki
char** (*createArray)(int);
void (*fillData)(char*, int);
void (*createNewBlock)(char **, int, int);
void (*deleteBlock)(char**, int, int);
void (*deleteArray)(char**);
int (*closestBlockTo_)(char*, char**, int);
int (*closestStaticBlockTo_)(char*, char[10000][10000], int);
char (*array)[10000][10000];


void pr_times(clock_t, struct tms *, struct tms *, FILE *);


int main(int argc, char* argv[]){
    //UCHWYTY itp
    //uchwyt do biblioteki
    handle = dlopen("./libmylibdynamic.so", RTLD_LAZY);
    
    char *error;
    //blad jesli nie zdolamym "chwycic" biblioteki
    if(!handle){
        fprintf(stderr, "Nie udalo sie z bib - %s\n", dlerror());
        return 1;
    }

    //uchwyty na funkcje:
    createArray = (char** (*)(int)) dlsym(handle, "createArray");
    error = dlerror();
    if(error != NULL){
        fprintf(stderr, "Nie udalo sie z createArray - %s\n", error);
        return 1;
    }

    fillData = (void (*)(char*, int)) dlsym(handle, "fillData");
    error = dlerror();
    if(error != NULL){
        fprintf(stderr, "%sNie udalo sie z fillData - \n", error);
        return 1;
    }

    createNewBlock = (void (*)(char**, int, int)) dlsym(handle, "createNewBlock");
    error = dlerror();
    if(error != NULL){
        fprintf(stderr, "Nie udalo sie z createNewBlock - %s\n", error);
        return 1;
    }

    deleteBlock = (void (*)(char**, int, int)) dlsym(handle, "deleteBlock");
    error = dlerror();
    if(error != NULL){
        fprintf(stderr, "Nie udalo sie z deleteBlock - %s\n", error);
        return 1;
    }

    deleteArray = (void (*)(char**)) dlsym(handle, "deleteArray");
    error = dlerror();
    if(error != NULL){
        fprintf(stderr, "Nie udalo sie z deleteArray - %s\n", error);
        return 1;
    }

    closestBlockTo_ = (int (*)(char*, char**, int)) dlsym(handle, "closestBlockTo_");
    error = dlerror();
    if(error != NULL){
        fprintf(stderr, "Nie udalo sie z closestBlockTo_ - %s\n", error);
        return 1;
    }

    closestStaticBlockTo_ = (int (*)(char*, char[10000][10000], int)) dlsym(handle, "closestStaticBlockTo_");
    error = dlerror();
    if(error != NULL){
        fprintf(stderr, "Nie udalo sie z closestStaticBlockTo_ - %s\n", error);
        return 1;
    }

    //chwytanie tablic
    array = (int***) dlsym(handle, "array");
    error = dlerror();
    if(error != NULL){
        fprintf(stderr, "Problem z static array - %s\n", error);
        return 1;
    }

    /******* PROGRAM *******/
    if(argc <= 4){
        printf("Za malo argumentow\n");
        return 0;
    }

    int size = atoi(argv[1]);
    if(size <= 0){
        printf("Rozmiar tablic jest niewlasciwy\n");
        return 0;
    }

    int blockSize = atoi(argv[2]);
    if(blockSize <= 0){
        printf("Rozmiar bloku jest niewlasciwy\n");
        return 0;
    }

    char ALLOCATION = argv[3][0];
    if(ALLOCATION != 's' && ALLOCATION != 'd'){
        printf("Typ allokacji zostal zle podany\n");
        return 0;
    }

    if(ALLOCATION == 's'){
        if(size > 10000 || blockSize > 10000){
            printf("Wielkosci tablicy i blokow dla tablicy alokowanej statycznie musza byc mniejsze od 10000 \n");
            return 0;
        }
    }

    //inicjacja rand()
    time_t t;
    srand((unsigned) time(&t));


    //otwarcie pliku
    FILE *fp;
    if((fp=fopen("raport3b_dynamic.txt", "w")) == NULL){
        printf("nie moge otworzyc pliku do zapisu\n");
        exit(1);
    }

    //wypisywanie podstawowych inforamcji do raportu
    if(ALLOCATION == 's')
        fprintf(fp, "Tablica statycznie alokowana\n");
    else
        fprintf(fp, "Tablica dynamicznie alokowana\n");

    fprintf(fp, "Wielkosc tablicy: %i \nRozmiar blokow: %i\n\n", size, blockSize);

    int numberOfOperations = argc - 4;

    char ** blockArray;

    int whichBlock;                         //ktory blok najblizszy

    //rzeczy odpowiedzialne za pomiar czasu
    struct tms tmsstart, tmsend;
    clock_t start, end;

    /*
     * operacje dostepne:
     * c - create array
     * f - wyszukanie elementu
     * mb - stworz nowy blok
     * db - usuniecie bloku
     * dmb - usuniecie a nastepnie stworzenie bloku
     */

    //tworzenie tablicy i wypelnianie wszystkich blokow
    if(argv[4][0] != 'c'){
        printf("Nie zostala stowrzona tablica\n");
        return 0;
    } else{
        printf("Operacja: Stworzenie tablicy\n");
        fprintf(fp, "Operacja: Stworzenie tablicy\n");

        start = times(&tmsstart);
        for(int i = 0; i < 50; i++){
            if(ALLOCATION == 'd'){
                blockArray = (*createArray)(size);
                for(int i = 0; i < size; i++)
                    (*createNewBlock)(blockArray, i, blockSize);
            }
            else{
                for (int i = 0; i <= size; ++i) {
                    (*fillData)((*array)[i], blockSize);
                }
            }
        }
        end = times(&tmsend);

        pr_times(end - start, &tmsstart, &tmsend, fp);
    }


    for(int i = 1; i < numberOfOperations; i++){
        if(argv[i+4][0] == 'c'){
            printf("Nie mozna zrobic dwoch tablic\n");
        }
        else if (argv[i+4][0] == 'f'){
            printf("Operacja: Znajdz najblizszy element\n");
            fprintf(fp, "Operacja: Znajdz najblizszy element\n");

            char character[20];
            for(int j = 0; j < 20; j++)
                character[j] = 0;

            printf("Wprowadz napis ktory chcesz porownac: ");
            scanf("%s", character);

            //rozpoczecie robienia polecenia
            start = times(&tmsstart);   //wartoscie poczatkowe
            for(int i = 0; i < 50; i++){
                if(ALLOCATION == 'd')
                    whichBlock = (*closestBlockTo_)(character, blockArray, size);
                else
                    whichBlock = (*closestStaticBlockTo_)(character, (*array), size);
            }         
            end = times(&tmsend);       //wartoscie koncowe

            pr_times(end - start, &tmsstart, &tmsend, fp);

            printf("Indeks bloku najbardziej podobnego: %i\n", whichBlock);
            fprintf(fp, "Indeks bloku najbardziej podobnego: %i\n", whichBlock);
        }
        else if(strcmp(argv[i+4], "mb") == 0){
            printf("Operacja: Stworz nowy blok\n");
            fprintf(fp, "Operacja: Stworz nowy blok\n");

            int ind;
            printf("Wybierz indeks: ");
            scanf("%i", &ind);

            if(ind < 0 || ind > size){
                printf("Zly indeks. Nie stworzono nowego bloku\n");
                continue;
            }

            start = times(&tmsstart);
            for(int j = 0; j < 50; j++){
                if(ALLOCATION == 's'){
                    (*fillData)((*array)[ind], blockSize);
                }
                else{
                    (*createNewBlock)(blockArray, ind, blockSize);
                }
            }
            end = times(&tmsend);

            pr_times(end - start, &tmsstart, &tmsend, fp);
        }
        else if(strcmp(argv[i+4], "db") == 0){
            printf("Operacja: Usun blok\n");
            fprintf(fp, "Operacja: Usun blok\n");

            if(ALLOCATION == 's'){
                printf("Operacja niedostepna dla statycznie alokowanej tablicy\n");
                continue;
            }

            int ind;
            printf("Wybierz indeks: ");
            scanf("%i", &ind);

            if(ind <= 0 || ind > size){
                printf("Zly indeks. Nie usunieto zadnego bloku\n");
                continue;
            }

            start = times(&tmsstart);
            for(int j = 0; j < 50; j++){
                (*deleteBlock)(blockArray, size, ind);
            }
            end = times(&tmsend);

            pr_times(end - start, &tmsstart, &tmsend, fp);
        }
        else if(strcmp(argv[i+4], "mdb") == 0){
            printf("Operacja: Tworz i usuwaj bloki\n");
            fprintf(fp, "Operacja: Tworz i usuwaj bloki\n");

            if(ALLOCATION == 's'){
                printf("Operacja niedostepna dla statycznie alokowanej tablicy\n");
                continue;
            }

            int ind, howMany;

            printf("Wybierz indeks: ");
            scanf("%i", &ind);
            if(ind < 0 || ind > size){
                printf("Zly indeks. Nie stworzono nowego bloku\n");
                continue;
            }

            printf("Wybierz ile razy ma byc dodawany i usuwany: ");
            scanf("%i", &howMany);
            if(howMany <= 0){
                printf("Ilosc wykonanych operacji musi byc dodatnia\n");
                continue;
            }

            start = times(&tmsstart);
            for(int j = 0; j < 50; j++){
                while(howMany > 0){
                    (*createNewBlock)(blockArray, ind, blockSize);
                    (*deleteBlock)(blockArray, size, ind);

                    howMany--;
                }
            }
            end = times(&tmsend);

            pr_times(end - start, &tmsstart, &tmsend, fp);
        }
        else{
            printf("Niepoprawna funckcja\n");
            continue;
        }

        printf("\n");
        fprintf(fp, "\n");
    }

    //zamykanie uchwytu
    dlclose(handle);

    //zamkniecie pliku
    fclose(fp);

    return 0;
}

//funkcja slozaca do wypisywania czasow wykonywania sie operacji
void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend, FILE *fp){
    static long clktck = 0;

    if(clktck == 0)
        if( (clktck = sysconf(_SC_CLK_TCK)) < 0){
            printf("sysconf error\n");
            return;
        }

    //wypisywanie czasu rzeczywistego
    printf("Real time: %7.6f\n", real / (double) (clktck * 50));
    fprintf(fp, "Real time: %7.6f\n", real / (double) (clktck * 50));

    //czas uzytkownika
    printf("User time: %7.6f\n",
           (tmsend->tms_utime - tmsstart->tms_utime) / (double) (clktck * 50));
    fprintf(fp, "User time: %7.6f\n",
           (tmsend->tms_utime - tmsstart->tms_utime) / (double) (clktck * 50));

    //czas systemowy
    printf("System time: %7.6f\n",
           (tmsend->tms_stime - tmsstart->tms_stime) / (double) (clktck * 50));
    fprintf(fp, "System time: %7.6f\n",
           (tmsend->tms_stime - tmsstart->tms_stime) / (double) (clktck * 50));

}