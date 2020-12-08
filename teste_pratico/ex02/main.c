/*
    Made by:
       João Ramalho, no. 95599
       Tomás Tavares, no. 95680
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "fs/operations.h"

#include "timer.h"
#include "circularBuffer.h"
#include <pthread.h>

#define FALSE 0
#define TRUE 1


/*Global mutex to control the synchronization of the access to the vector of commands */
pthread_mutex_t commandsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canProduce = PTHREAD_COND_INITIALIZER;
pthread_cond_t canConsume = PTHREAD_COND_INITIALIZER;

/* Number of consumer threads */
int numberThreads = 0;

CircularBuffer commandsBuffer;

/* Flag that has the state of the file of inputs (ended or not) if possible */
int endOfFile = 0;

/* Inserts the command [data] in the buffer */
int insertCommand(char* data, CircularBuffer *buffer) {

    if(buffer->numberElements != MAX_COMMANDS) {
        addInput(buffer, data);
        return 1;
    }
    return 0;
}

/* Removes first command introduced in the buffer if possible */
char* removeCommand(CircularBuffer *buffer) {
    char* command;
    
    if(buffer->numberElements > 0){
        command = removeHead(buffer); 
        return command;  
    }

    return NULL;
}


void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

/* Similar to "errorParse" but prints custom messages */
void errorParseCustom(char *error) {
    fprintf(stderr, "Error: %s\n", error);
    exit(EXIT_FAILURE);
}

/** Given a file reads the first line checks for errors and if 
*   there arent none adds the command to the buffer 
*/
void processInput(FILE *fpRead){
    char line[MAX_INPUT_SIZE];
    
    /* Read a line from the file */
    if (fgets(line, sizeof(line)/sizeof(char), fpRead) == NULL) {
        endOfFile = TRUE;
        return;
    }

    char token;
    char name[MAX_INPUT_SIZE], typeOrPath[MAX_INPUT_SIZE];

    int numTokens = sscanf(line, "%c %s %s", &token, name, typeOrPath);

    /* perform minimal validation */
    if (numTokens < 1) {
        return;
    }

    /** Check if the line read is a valid one if it is 
    *         adds it to the commandsBuffer 
    */
    switch (token) {
        case 'c':
            if(numTokens != 3)
                errorParse();
            if(insertCommand(line, &commandsBuffer))
                break;
            return;
        
        case 'l':
            if(numTokens != 2)
                errorParse();
            if(insertCommand(line, &commandsBuffer))
                break;
            return;
        
        case 'd':
            if(numTokens != 2)
                errorParse();
            if(insertCommand(line, &commandsBuffer))
                break;
            return;
        
        case 'm' :
            if(numTokens != 3)
                errorParse();
            if(insertCommand(line, &commandsBuffer))
                break;
            return;
        
        case '#':
            break;
        
        default: { /* error */
            errorParse();
        }
    }
}

/* Receives a command and process the corresponding action */
void applyCommands(char *command){
    int searchResult;
    
    if (command == NULL){
        return;
    }

    char token, type;
    char name[MAX_INPUT_SIZE];
    char typeOrPath[MAX_INPUT_SIZE];

    /* Splits the command in parts [token, name, type/path]*/
    int numTokens = sscanf(command, "%c %s %s", &token, name, typeOrPath);
    if (numTokens < 2) {
        errorParseCustom("invalid command in Queue");
    }

    /* With all the parts of the command executes the corresponding action */
    switch (token) {
        case 'c':
            type = typeOrPath[0];
            switch (type) {
                case 'f':
                    printf("Create file: %s\n", name);

                    create(name, T_FILE);
                    break;
                case 'd':
                    printf("Create directory: %s\n", name);
                    
                    create(name, T_DIRECTORY);
                    break;
                default:
                    errorParseCustom("invalid node type");
            }
            break;
        case 'l':     
            searchResult = lookup(name);

            if (searchResult >= 0){
                printf("Search: %s found\n", name);
            }
            else{
                printf("Search: %s not found\n", name);
            }
            break;
        case 'd':
            printf("Delete: %s\n", name);
            
            delete(name);
            break;
        case 'm':
            printf("Move: %s to %s\n", name, typeOrPath);
            move(name, typeOrPath);

            break;
        default: { /* error */
            errorParseCustom("command to apply");
        }
    }
}

/* Predicate to check if there are still commands to be processed */
int keepProducing() {
    int condition;
    pthread_mutex_lock(&commandsMutex);
    condition = (endOfFile == FALSE);
    pthread_mutex_unlock(&commandsMutex);
    
    return condition;
}

/**  Function that "produce commands". If the buffer isnt full,
*    gets the first command of the file fpRead and adds it to
*           the buffer of commands to be consumed
*/
void *fnProduce(void *arg) {
    FILE *fpRead = (FILE *) arg;
    
    while ( keepProducing() ) {
        pthread_mutex_lock(&commandsMutex);
        /* If the buffer is full wait until it isnt */
        while (isBufferFull(commandsBuffer) == TRUE) {
            pthread_cond_wait(&canProduce, &commandsMutex);
        }
        
        processInput(fpRead);

        /* The command was add so the consumers can now consume */
        pthread_cond_broadcast(&canConsume);
        pthread_mutex_unlock(&commandsMutex);
    }

    return NULL;
}

/* Predicate to check if there are still commands to be executed in the buffer */
int keepConsuming() {
    int condition;
    pthread_mutex_lock(&commandsMutex);
    condition = ((endOfFile == FALSE) || (isBufferEmpty(commandsBuffer) == FALSE));
    pthread_mutex_unlock(&commandsMutex);
    
    return condition;
}
/**  Function that "consumes commands". If the buffers isnt 
 *   empty, gets the command that was inserted first in the
 *          buffer and executes his functionality 
 */
void *fnConsume(void *arg) {
    char command[MAX_INPUT_SIZE];

    while ( keepConsuming() ) {
        pthread_mutex_lock(&commandsMutex);
        /* If the buffer is empty wait until it isnt */
        while (isBufferEmpty(commandsBuffer) == TRUE) {
            /* Checks if there is nothing else to be produced */
            if (endOfFile == TRUE) {
                pthread_cond_broadcast(&canConsume);
                pthread_mutex_unlock(&commandsMutex);
                return NULL;   
            }

            pthread_cond_wait(&canConsume, &commandsMutex);
        }

        /* Get the command from the buffer */
        strcpy(command, removeCommand(&commandsBuffer));
        
        /* The command was consumed so the producer can add a new one */
        pthread_cond_signal(&canProduce);
        pthread_mutex_unlock(&commandsMutex);
        
        applyCommands(command);
    }

    return NULL;
}
/** Allocates memory for the string passed by argument
*    returning a pointer to the allocated memory */
char *stringCopy(char *string) {
    char *newString;

    newString = (char *) malloc((sizeof(char) * strlen(string)) + 1);
    strcpy(newString, string);

    return newString;
}

/* Frees all allocated memory */
void closeProgram(FILE *fpRead, FILE *fpOut, char *inputFile, char *outputFile) {
    /* release allocated memory */
    destroy_fs();

    /*close previously opened files*/
    fclose(fpRead);
    fclose(fpOut);

    /* release allocated memory */
    free(inputFile);
    free(outputFile);

    pthread_mutex_destroy(&commandsMutex);
    pthread_cond_destroy(&canProduce);
    pthread_cond_destroy(&canConsume);
    exit(EXIT_SUCCESS);

}

int main(int argc, char* argv[]) {
    initBuffer(&commandsBuffer);

    FILE *fpRead;
    FILE *fpOut;

    Timer timer;

    /* init filesystem */
    init_fs();
    
    /* Error : Invalid number of arguments */
    if (argc != 4) {
        errorParseCustom("Invalid number of arguments");
    }

    char *inputFile = stringCopy(argv[1]);
    char *outputFile = stringCopy(argv[2]);

    numberThreads = atoi(argv[3]);
    
    /* Error : numberThreads not an int or <= 0 */
    if (numberThreads <= 0) {
        errorParseCustom("numberThreads not an int or <= 0");
    }

    /* Open the input file */
    fpRead = fopen(inputFile, "r");

    /* Error : Check if the input file is valid */
    if (fpRead == NULL) {
        errorParseCustom("Check if the input file is valid");
    }

    /* Open the output file */
    fpOut = fopen(outputFile, "w");

    /* Error : Check if the output file is valid */
    if (fpOut == NULL) {
        errorParseCustom("Check if the output file is valid");
    }

    /* Create thread pool and execute commands */
    pthread_t tidConsumer[numberThreads];
    pthread_t tidProducer;

    /* Start timer*/
    startTimer(&timer);
    
    /* Create thread pool */
    if (pthread_create(&tidProducer, NULL, fnProduce, fpRead) != 0)
        errorParseCustom("Failed to create thread");

    for (int i = 0; i < numberThreads; i++) {
        if(pthread_create(&tidConsumer[i], NULL, fnConsume, NULL) != 0)
            errorParseCustom("Failed to create thread");
    }

    
    /* Waiting for all the commands to be executed */
    if (pthread_join(tidProducer, NULL) != 0)
        errorParseCustom("Failed to join thread");

    for (int i = 0; i < numberThreads; i++) {
        if(pthread_join(tidConsumer[i], NULL) != 0)
            errorParseCustom("Failed to join thread");
    }

    /* Stop timer */
    stopTimer(&timer);


    print_tecnicofs_tree(fpOut);

    printf("TecnicoFS completed in %.4f seconds.\n", timer.elapsedTime);
    closeProgram(fpRead, fpOut, inputFile, outputFile);
}
