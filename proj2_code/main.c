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


/*Global mutex to control the synchronization of the acess to the vector of commands */
pthread_mutex_t commandsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canProduce = PTHREAD_COND_INITIALIZER;
pthread_cond_t canConsume = PTHREAD_COND_INITIALIZER;

int numberThreads = 0;

CircularBuffer buffer;

/* Flag that has the state of the file of inputs (ended or not) */
int endOfFile = 0;


int insertCommand(char* data) {

    if(buffer.numberElements != MAX_COMMANDS) {
        addInput(&buffer, data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    char* command;

    if(buffer.numberElements > 0){
        command = removeHead(&buffer); 
        return command;  
    }

    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void errorParseCustom(char *error) {
    fprintf(stderr, "Error: %s\n", error);
    exit(EXIT_FAILURE);
}

void processInput(FILE *fpRead){
    char line[MAX_INPUT_SIZE];
    
    /* break loop with ^Z or ^D */
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

    switch (token) {
        case 'c':
            if(numTokens != 3)
                errorParse();
            if(insertCommand(line))
                break;
            return;
        
        case 'l':
            if(numTokens != 2)
                errorParse();
            if(insertCommand(line))
                break;
            return;
        
        case 'd':
            if(numTokens != 2)
                errorParse();
            if(insertCommand(line))
                break;
            return;
        
        case 'm' :
            if(numTokens != 3)
                errorParse();
            if(insertCommand(line))
                break;
            return;
        
        case '#':
            break;
        
        default: { /* error */
            errorParse();
        }
    }
}

void applyCommands(FILE *fpOut){
    const char* command = removeCommand();

    if (command == NULL){
        return;
    }

    char token, type;
    char name[MAX_INPUT_SIZE];
    char typeOrPath[MAX_INPUT_SIZE];

    int numTokens = sscanf(command, "%c %s %s", &token, name, typeOrPath);
    if (numTokens < 2) {
        fprintf(stderr, "Error: invalid command in Queue\n");
        exit(EXIT_FAILURE);
    }

    int searchResult;
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
                    fprintf(stderr, "Error: invalid node type\n");
                    exit(EXIT_FAILURE);
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
            fprintf(stderr, "Error: command to apply\n");
            exit(EXIT_FAILURE);
        }
    }
}

int keepProducing() {
    int condition;
    pthread_mutex_lock(&commandsMutex);
    condition = (endOfFile == FALSE);
    pthread_mutex_unlock(&commandsMutex);
    
    return condition;
}

void *fnProduce(void *arg) {
    FILE *fpRead = (FILE *) arg;
    
    while ( keepProducing() ) {
        pthread_mutex_lock(&commandsMutex);
        while (isBufferFull(buffer) == TRUE) {
            pthread_cond_wait(&canProduce, &commandsMutex);
        }
        
        processInput(fpRead);
        pthread_cond_broadcast(&canConsume);

        pthread_mutex_unlock(&commandsMutex);
    }

    return NULL;
}

int keepConsuming() {
    int condition;
    pthread_mutex_lock(&commandsMutex);
    condition = ((endOfFile == FALSE) || (isBufferEmpty(buffer) == FALSE));
    pthread_mutex_unlock(&commandsMutex);
    
    return condition;
}

void *fnConsume(void *arg) {
    FILE *fpOut = (FILE *) arg;

    while ( keepConsuming() ) {
        pthread_mutex_lock(&commandsMutex);
        while (isBufferEmpty(buffer) == TRUE) {
            /* Perguntar ao stor */
            if (endOfFile == TRUE) {
                pthread_cond_broadcast(&canConsume);
                pthread_mutex_unlock(&commandsMutex);
                return NULL;   
            }

            pthread_cond_wait(&canConsume, &commandsMutex);
        }

        applyCommands(fpOut);
        pthread_cond_signal(&canProduce);

        pthread_mutex_unlock(&commandsMutex);
    }

    return NULL;
}

char *stringCopy(char *string) {
    /* Allocates memory for the string passed by argument */
    char *newString;

    newString = (char *) malloc((sizeof(char) * strlen(string)) + 1);
    strcpy(newString, string);

    return newString;
}

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
    exit(EXIT_SUCCESS);

}

int main(int argc, char* argv[]) {
    initBuffer(&buffer);

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
        if(pthread_create(&tidConsumer[i], NULL, fnConsume, fpOut) != 0)
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
