/*
 * Tworca: Tomasz Karkocha
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/times.h>

void generate(char* fileName, int howManyRecords, int recordLength);
void sortSys(char* fileName, int howManyRecords, int recordLength);
void sortLib(char* fileName, int howManyRecords, int recordLength);
void copySys(char* fileFrom, char* fileTo, int howManyRecords, int recordLength);
void copyLib(char* fileFrom, char* fileTo, int howManyRecords, int recordLength);
void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend);
void wypiszWlasciwosci(int a, int b);


int main(int argc, char* argv[]) {
    //inicjacjacja do losowania
    time_t tt;
    srand(time(&tt));

    //rzeczy do pomiaru czasu
    struct tms tmsstart, tmsend;
    clock_t start, end;

    //string trzymajacy nazwe funkcji
    char *func = argv[1];

    if(strcmp(func, "generate") == 0){      //generowanie
        char *fileName = argv[2];
        int records = atoi(argv[3]);
        int recordSize = atoi(argv[4]);

        if(records < 1){
            printf("Liczba rekordow musi byc dodatnia");
            return 1;
        }

        if(recordSize < 1){
            printf("Wielkosc rekordow musi byc dodatnia");
            return 1;
        }

        printf("Generowanie %i rekordow o rozmiarze: %i\n", records, recordSize);
        generate(fileName, records, recordSize);
    }
    else if(strcmp(func, "sort") == 0){     //sortowanie
        char *fileName = argv[2];
        int records = atoi(argv[3]);
        int recordSize = atoi(argv[4]);
        char *whichFucnType = argv[5];

        start = times(&tmsstart);
        if(strcmp(whichFucnType, "sys") == 0) {
            printf("==== Sortowanie przy pomocy funkcji systemowych ====\n");
            wypiszWlasciwosci(records, recordSize);
            sortSys(fileName, records, recordSize);
        }
        else if(strcmp(whichFucnType, "lib") == 0) {
            printf("==== Sortowanie przy pomocy funkcji bibliotecznych ====\n");
            wypiszWlasciwosci(records, recordSize);
            sortLib(fileName, records, recordSize);
        }
        else{
            printf("zlu argument\n");
            return 1;
        }
        end = times(&tmsend);

        pr_times(end - start, &tmsstart, &tmsend);
    }
    else if(strcmp(func, "copy") == 0){     //kopiowanie
        char *fileFrom = argv[2];
        char *fileTo = argv[3];
        int records = atoi(argv[4]);
        int recordSize = atoi(argv[5]);
        char *whichFucnType = argv[6];

        start = times(&tmsstart);
        if(strcmp(whichFucnType, "sys") == 0) {
            printf("==== Kopiowanie przy pomocy funkcji systemowych ====\n");
            wypiszWlasciwosci(records, recordSize);
            copySys(fileFrom, fileTo, records, recordSize);
        }
        else if(strcmp(whichFucnType, "lib") == 0) {
            printf("==== Kopiowanie przy pomocy funkcji bibliotecznych ====\n");
            wypiszWlasciwosci(records, recordSize);
            copyLib(fileFrom, fileTo, records, recordSize);
        }
        else{
            perror("Zly argument\n");
            return 1;
        }
        end = times(&tmsend);

        pr_times(end - start, &tmsstart, &tmsend);
    }
    else{
        perror("Zla funkcja\n");
        return 1;
    }

    printf("\n");

    return 0;
}

// ===========FUNKCJE=========== //

void wypiszWlasciwosci(int a, int b){
    printf("Liczba rekordow: %i\tRozmiar rekordow: %i\n", a, b);
}

void generate(char* fileName, int howManyRecords, int recordLength){
    //otwarcie pliku
    int wy = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    if(wy == NULL){
        perror("Nie udalo sie otworzyc pliku");
        exit(1);
    }

    //zalokowanie tablicy
    unsigned char tab[recordLength];

    int liczba = 0;

    //wypelnianie tablicy
    for(int i = 0; i < howManyRecords; i++){
        for (int j = 0; j < recordLength; ++j) {
            tab[j] = ((rand() % (126 - 33)) + 33);
        }

        liczba += write(wy, tab, recordLength);
    }

    printf("Ilosc bajtow zapisanych do pliku: %i\n", liczba);

    close(wy);
}

int comparision(char* x, char* klucz, int recordLength){
    for(int i = 0; i < recordLength; i++){
        if(x[i] > klucz[i])
            return 1;
        else if(x[i] < klucz[i])
            return 0;
    }

    return 0;
}

// ==== SORTOWANIE ==== //
void sortSys(char* fileName, int howManyRecords, int recordLength){
    int we = open(fileName, O_WRONLY);
    int wy = open(fileName, O_RDONLY);

    if(wy == NULL){
        perror("Nie udalo sie otworzyc pliku");
        exit(1);
    }
    if(we == NULL){
        perror("Nie udalo sie otworzyc pliku");
        exit(1);
    }

    //tablice do przechowywania rekordow
    char rec1[recordLength], rec2[recordLength];

    //ilosc symboli
    int size = howManyRecords * recordLength;
    int j;

    for (int i = recordLength; i < size; i= i + recordLength) {
        //za pomoca funkcji lseek chodzimy po pliku
        lseek(wy, i, SEEK_SET);
        read(wy, rec2, recordLength);

        j = i - recordLength;

        lseek(wy, j, SEEK_SET);
        read(wy, rec1, recordLength);

        while (j >= 0 && comparision(rec1, rec2, recordLength)){
            lseek(wy, j, SEEK_SET);
            read(wy, rec1, recordLength);

            lseek(we, j + recordLength, SEEK_SET);
            write(we, rec1, recordLength);

            j = j - recordLength;

            lseek(we, j + recordLength, SEEK_SET);
            write(we, rec2, recordLength);

            lseek(wy, j, SEEK_SET);
            read(wy, rec1, recordLength);
        }
    }

    close(we);
    close(wy);
}

void sortLib(char* fileName, int howManyRecords, int recordLength){
    FILE *fpr = fopen(fileName, "r");
    FILE *fpw = fopen(fileName, "r+");

    if(fpr == NULL || fpw == NULL){
        perror("Nie udalo sie polaczyc do pliku!\n");
        exit(1);
    }

    //tablice do przechowywania rekordow
    char rec1[recordLength], rec2[recordLength];

    int size = howManyRecords * recordLength;
    int j;

    for(int i = recordLength; i < size; i = i + recordLength){
        fseek(fpr, i, 0);
        fread(rec2, sizeof(char), recordLength, fpr);

        j = i - recordLength;

        fseek(fpr, j, 0);
        fread(rec1, sizeof(char), recordLength, fpr);

        while (j >= 0 && comparision(rec1, rec2, recordLength)){
            fseek(fpr, j, 0);
            fread(rec1, sizeof(char), recordLength, fpr);

            fseek(fpw, j + recordLength, 0);
            fwrite(rec1, sizeof(char), recordLength, fpw);

            j = j - recordLength;

            fseek(fpw, j + recordLength, 0);
            fwrite(rec2, sizeof(char), recordLength, fpw);

            fseek(fpr, j, 0);
            fread(rec1, sizeof(char), recordLength, fpr);
        }

    }

    fclose(fpr);
    fclose(fpw);
}


// ==== KOPIOWANIE ==== //
void copySys(char* fileFrom, char* fileTo, int howManyRecords, int recordLength){
    int fFrom = open(fileFrom, O_RDONLY);
    int fTo = open(fileTo, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

    if(fFrom == NULL || fTo == NULL){
        perror("Nie udalo sie polaczyc do pliku\n");
        exit(1);
    }

    char buf[recordLength];


    for (int i = 0; i < howManyRecords; i++) {

        read(fFrom, buf, recordLength);

        write(fTo, buf, recordLength);
    }

    close(fTo);
    close(fFrom);
}

void copyLib(char* fileFrom, char* fileTo, int howManyRecords, int recordLength){
    FILE *fFrom = fopen(fileFrom, "r");
    FILE *fTo = fopen(fileTo, "w");

    if(fFrom == NULL || fTo == NULL){
        perror("Nie udalo sie polaczyc do pliku\n");
        exit(1);
    }

    char buf[recordLength];

    for (int i = 0; i < howManyRecords; ++i) {
        fread(buf, sizeof(char), recordLength, fFrom);

        fwrite(buf, sizeof(char), recordLength, fTo);
    }

    fclose(fFrom);
    fclose(fTo);
}



// ==== MIERZENIE CZASU ==== //
void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend){
    static long clktck = 0;

    if(clktck == 0)
        if( (clktck = sysconf(_SC_CLK_TCK)) < 0){
            printf("sysconf error\n");
            return;
        }

    //wypisywanie czasu rzeczywistego
    printf("Real time: %7.6f\n", real / (double) (clktck));

    //czas uzytkownika
    printf("User time: %7.6f\n",
           (tmsend->tms_utime - tmsstart->tms_utime) / (double) (clktck));

    //czas systemowy
    printf("System time: %7.6f\n",
           (tmsend->tms_stime - tmsstart->tms_stime) / (double) (clktck));

}

