/*
 * Autor: Tomasz Karkocha
 * Library: charBlockArray
 * -----------------------
 * This library is used to create an array
 * which points to blocks of chars.
 *
 */

#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

char array[10000][10000];

// function which create an array without allocating blocks
char** createArray(int size){
    if(size <= 0){
        printf("The size of array must be larger than 0!\n");
        exit(0);
    }

    char** blockArray;

    blockArray = (char **)calloc(size, sizeof(char**));

    if(blockArray == NULL){
        printf("Problem with allocation. Aborting...\n");
        exit(0);
    }

    return blockArray;
}


// function which delete array
void deleteArray(char** blockArray){
    free(blockArray);
    blockArray = 0;
}


// function generarting random data into blocks
void fillData(char* blockArray, int blockSize){
    // 33(!) - 126(~) wszystkie znaki w ascii
    for(int i = 0; i < blockSize; i++){
        blockArray[i] = (rand() % (126 - 33)) + 33;
    }
}


//function creating a new block
void createNewBlock(char** blockArray, int index, int blockSize){
    if(blockArray == NULL){
        printf("Array is pointing on NULL!\n");
        exit(0);
    }

    if(index >= blockSize || index < 0){
        printf("Index is invalid!\n");
        exit(0);
    }

    if(blockSize <= 0){
        printf("Size of block must be greater than 0!\n");
        exit(0);
    }

    blockArray[index] = (char*)calloc(blockSize, sizeof(char*));

    //wypelniamy blok losowymi danymi
    fillData(blockArray[index], blockSize);
}


//function deleting a block
void deleteBlock(char** blockArray, int size, int index){
    if(index >= size || index < 0){
        printf("Index is invalid!\n");
        exit(0);
    }

    // calling free() on a null will do nothing
    free(blockArray[index]);
    blockArray[index] = 0;
}

//function which find block wchich is neares to an character
int closestBlockTo_(char *character, char** blockArray, int size){
    int index = -1;
    int tmp = 1000000;
    int sumChar = 0;

    for(int i = 0; i < 20; i++){
        sumChar += (int)character[i];
    }

    for(int i = 0; i < size; i++){
        int sum = 0;
        int j = 0;

        do{
          sum += (int)blockArray[i][j];
          j++;
        } while(blockArray[i][j] != 0);

        if(abs(sumChar - sum) < tmp){
            tmp = sum;
            index = i;
        }

    }

    return index;
}

int closestStaticBlockTo_(char *character, char array[10000][10000], int size){
    int index = -1;
    int tmp = 1000000;
    int sumChar = 0;

    for(int i = 0; i < 20; i++){
        sumChar += (int)character[i];
    }

    for(int i = 0; i < size; i++){
        int sum = 0;
        int j = 0;

        do{
            sum += (int)array[i][j];
            j++;
        } while(array[i][j] != 0);

        if(abs(sumChar - sum) < tmp){
            tmp = sum;
            index = i;
        }

    }

    return index;
}
