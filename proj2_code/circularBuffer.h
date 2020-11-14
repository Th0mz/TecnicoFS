#ifndef _CIRCULARBUFFER_
#define _CIRCULARBUFFER_

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

#include <pthread.h>

/* CIRCULAR BUFFER STRUCTURE*/
typedef struct circBuf {
    char inputElements[MAX_COMMANDS][MAX_INPUT_SIZE];
    int head, tail;
    int numberElements;
} CircularBuffer;


/* FUNCTIONS */
void initBuffer(CircularBuffer *buffer);
void addInput(CircularBuffer *buffer, char *lineInput);
char* removeHead(CircularBuffer *buffer);
int isBufferFull(CircularBuffer buffer);
int isBufferEmpty(CircularBuffer buffer);

#endif