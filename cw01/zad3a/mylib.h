#ifndef LAB1_LIBRARY_H
#define LAB1_LIBRARY_H

char** createArray(int size);
void deleteArray(char** blockArray);
void fillData(char* array, int blockSize);
void createNewBlock(char** blockArray, int index, int blockSize);
void deleteBlock(char** blockArray, int size ,int index);
int closestBlockTo_(char *character, char** blockArray, int size);
int closestStaticBlockTo_(char *character, char array[10000][10000], int size);

#endif
