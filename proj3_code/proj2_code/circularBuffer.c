#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "circularBuffer.h"

/* Initializes the circular buffer */
void initBuffer(CircularBuffer *buffer) {
    buffer->head = 0;
    buffer->tail = 0;

    buffer->numberElements= 0;
}

/* Adds a new element to the buffer */
void addInput(CircularBuffer *buffer, char *lineInput) {
    int tail = buffer->tail;
    strcpy(buffer->inputElements[tail], lineInput);
    
    buffer->numberElements++;

    tail = (tail+1) % MAX_COMMANDS;
    buffer->tail = tail;
}

/* Removes the fist element inserted in the buffer */
char* removeHead(CircularBuffer *buffer) {
    int head = buffer->head;
    char *element = buffer->inputElements[head];
    
    buffer->numberElements--;

    head = (head + 1) % MAX_COMMANDS;
    buffer->head = head;

    return element;
}

/* Checks if the buffer is full */
int isBufferFull(CircularBuffer buffer) {
    if (buffer.numberElements == MAX_COMMANDS)
        return 1;
    else
        return 0;
}

/* Checks if the buffer is empty */
int isBufferEmpty(CircularBuffer buffer) {
    if (buffer.numberElements == 0)
        return 1;
    else
        return 0;
}