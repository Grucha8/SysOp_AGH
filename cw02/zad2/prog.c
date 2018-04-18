//
// Created by tkarkocha on 21.03.18.
//
#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <values.h>

#define FTW_F 1     //plik nie bedacy katalogiem
#define FTW_D 2     //katalog
#define FTW_DNR 3   //katalog ktory nie moze byc czytany
#define FTW_NS 4    //plik dla ktorego nie mozna wykonac stat

//oznaczenia dla drugiego argumentu
#define LESS 1
#define EQUAL 2
#define GREATER 3

static time_t currentTime;
static int modyfication;

static void myftw(char *pathname);
static int doPath();
static int myFunc(const char* pathname, const struct stat *statptr, int type);
static void wypisz(const char* pathname, const struct stat *statptr);
static int myNftwFunc(const char *pathname, const struct stat *statptr,
                     int tflag, struct FTW *ftwbuf);

int main(int argc, char* argv[]){
    //przeksztalcanie daty
    argv[3][4] = argv[3][7] = '\0';
    struct tm tmdate = {0};
    tmdate.tm_year = atoi(&argv[3][0]);;
    tmdate.tm_mon = atoi(&argv[3][5]);
    tmdate.tm_mday = atoi(&argv[3][8]);
    currentTime = mktime( &tmdate );


    if(strcmp(argv[2], "<") == 0){
        modyfication = LESS;
    } else if (strcmp(argv[2], "=") == 0){
        modyfication = EQUAL;
    } else if (strcmp(argv[2], ">") == 0){
        modyfication = GREATER;
    } else {
        printf("zly operator porownania\n");
        return 1;
    }

    if(strcmp(argv[4], "nftw") == 0) {
        //ustawiamy flagi
        int flags = 0;
        flags |= FTW_PHYS;          //nie wchodzimy w linki

        nftw(argv[1], myNftwFunc, 20, flags);
    }
    else if(strcmp(argv[4], "def") == 0)
        myftw(argv[1]);
    else{
        printf("Zly ostatni argument\n");
        return 1;
    }

    return 0;
}


// ==== FUNKCJE ==== //
static char* fullPath;

static void myftw(char *pathname){
    fullPath = calloc(PATH_MAX+1, sizeof(char));

    strcpy(fullPath, pathname);

    doPath();
}

static int doPath(){
    struct stat     statBuf;
    struct dirent*  dirp;
    DIR*            dp;
    int             ret;
    char*           ptr;

    //nie da sie wczytac stat
    if(lstat(fullPath, &statBuf) < 0){
        return (myFunc(fullPath, &statBuf, FTW_NS));
    }

    //sprawdzanie czy to jest plik
    if(S_ISDIR(statBuf.st_mode) == 0)
        return (myFunc(fullPath, &statBuf, FTW_F));

    //teraz juz wiemy ze to jest katalog
    if( (ret = myFunc(fullPath, &statBuf, FTW_D)) != 0){
        return ret;
    }


    ptr = fullPath + strlen(fullPath);      //robimy by nasz wskaznik wskazywal na koniec stringa
    *ptr++ = '/';                           //koniec byl zakonczony '/'
    *ptr = 0;

    //gdy nie da sie czytac katalogu
    if( (dp = opendir(fullPath)) == NULL){
        return myFunc(fullPath, &statBuf, FTW_DNR);
    }

    //petla rekurencyjna
    while( (dirp = readdir(dp)) != NULL){
        if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0){
            continue; //ifnorujemy katalogi . oraz ..
        }

        /*
         * Jako ze ptr wskazuje na koniec stringa ktory
         * ma w sobie sciezke do obecnego katalogu
         * i jest zakonczony /, mozemy przekopiowac
         * tam gdzie ptr wskazuje nazwe katalogu
         * i wtedy fullPath bedzie wskazywal na ten katalog
         */
        strcpy(ptr, dirp->d_name);

        if( (ret = doPath()) != 0)
            break;
    }
    ptr[-1] = 0;

    if(closedir(dp) < 0){
        printf("Nie mozna zamknac katalogu\n");
        exit(1);
    }

    return ret;
}

static int myFunc(const char* pathname, const struct stat *statptr, int type){
    //jesli pliku nie da sie odczytac to nic z nim nie rob
    if(type == FTW_DNR || type == FTW_NS) return 0;

    //gdy mamy do czynnienia z symbolic link lub z blokiem
    if( S_ISLNK(statptr->st_mode) || S_ISBLK(statptr->st_mode))
        return 0;


    switch (modyfication) {
        case LESS:
            if(difftime(currentTime, statptr->st_mtime) < 0){
                wypisz(pathname, statptr);
            }
            break;
        case EQUAL:
            if(difftime(currentTime, statptr->st_mtime) == 0){
                wypisz(pathname, statptr);
            }
            break;
        case GREATER:
            if(difftime(currentTime, statptr->st_mtime) > 0){
                wypisz(pathname, statptr);
            }
            break;
        default:
            printf("zly operator porownianai\n");
            exit(1);
    }

    return 0;

}

//zwraca stringa reprezentujacego poziomy dostepu
static char* getAccess(mode_t tmp){
    char* access = calloc(9, sizeof(char));

    //uzytkownik
    if((tmp & S_IRUSR) == 0)
        access[0] = '-';
    else
        access[0] = 'r';
    if((tmp & S_IWUSR) == 0)
        access[1] = '-';
    else
        access[1] = 'w';
    if((tmp & S_IXUSR) == 0)
        access[2] = '-';
    else
        access[2] = 'x';

    //grupa
    if((tmp & S_IRGRP) == 0)
        access[3] = '-';
    else
        access[3] = 'r';
    if((tmp & S_IWGRP) == 0)
        access[4] = '-';
    else
        access[4] = 'w';
    if((tmp & S_IXGRP) == 0)
        access[5] = '-';
    else
        access[5] = 'x';

    //inni
    if((tmp & S_IROTH) == 0)
        access[6] = '-';
    else
        access[6] = 'r';
    if((tmp & S_IWOTH) == 0)
        access[7] = '-';
    else
        access[7] = 'w';
    if((tmp & S_IXOTH) == 0)
        access[8] = '-';
    else
        access[8] = 'x';

    return access;

}

static void wypisz(const char* pathname, const struct stat *statptr){
    //wydobycie stringa reprezentujacego dostepnosc
    char* access = getAccess(statptr->st_mode);

    //stworzenie stringa pokazujacego czas ostatniej modyfikacji
    char* buff = calloc(20, sizeof(char));
    struct tm * timeInfo;
    timeInfo = localtime(&(statptr->st_mtime));
    strftime(buff, 20, "%y-%m-%d", timeInfo);

    //wypisujemy
    printf("%-9s  %-8s  %15jd  %s\n", access, buff, statptr->st_size, pathname);

    free(buff);
    free(access);
}


//blizniacza funkcja co ta wyzej ale musi zwracac 0 by nftw dziala poprawnie
static int myNftwFunc(const char *pathname, const struct stat *statptr,
                  int tflag, struct FTW *ftwbuf){

    if(S_ISLNK(statptr->st_mode) || S_ISBLK(statptr->st_mode))
        return 0;

    char* access = getAccess(statptr->st_mode);

    char* buff = calloc(20, sizeof(char));
    struct tm * timeInfo;
    timeInfo = localtime(&(statptr->st_mtime));
    strftime(buff, 20, "%y-%m-%d", timeInfo);


    printf("%-9s  %-8s  %15jd  %s\n", access, buff, statptr->st_size, pathname);

    return 0;
}

