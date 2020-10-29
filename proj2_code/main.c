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
#include <pthread.h>

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

/*Global mutex to control the synchronization of the acess to the vector of commands */
pthread_mutex_t commandsMutex = PTHREAD_MUTEX_INITIALIZER;

int numberThreads = 0;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;


int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    
    pthread_mutex_lock(&commandsMutex);

    if(numberCommands > 0){
        numberCommands--;

        pthread_mutex_unlock(&commandsMutex);
        /*FIXME headQueue is shared memory*/
        return inputCommands[headQueue++];  
    }

    pthread_mutex_unlock(&commandsMutex);

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
    while (fgets(line, sizeof(line)/sizeof(char), fpRead)) {
        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %c", &token, name, &type);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
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
            
            case '#':
                break;
            
            default: { /* error */
                errorParse();
            }
        }
    }
}

void applyCommands(FILE *fpOut){
    while (numberCommands > 0){
        const char* command = removeCommand();
        if (command == NULL){
            continue;
        }

        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        switch (token) {
            case 'c':
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
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void *fnThread(void *arg) {
    applyCommands(stdout);

    return NULL;
}

char *stringCopy(char *string) {
    /* Allocates memory for the string passed by argument */
    char *newString;

    newString = (char *) malloc((sizeof(char) * strlen(string)) + 1);
    strcpy(newString, string);

    return newString;
}

int main(int argc, char* argv[]) {
    /* init filesystem */
    FILE *fpRead;
    FILE *fpOut;

    Timer timer;

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

    /* Set the path for the input file */
    fpRead = fopen(inputFile, "r");

    /* Error : Check if the input file is valid */
    if (fpRead == NULL) {
        errorParseCustom("Check if the input file is valid");
    }


    /* Set the path for the output file */
    fpOut = fopen(outputFile, "w");

    /* Error : Check if the output file is valid */
    if (fpOut == NULL) {
        errorParseCustom("Check if the output file is valid");
    }
    
    /* process input and print tree */
    processInput(fpRead);

    /* Create thread pool and execute commands */
    pthread_t tid[numberThreads];
    
    /* Start timer*/
    startTimer(&timer);

    /*Reservar tarefa para ler do buffer*/
    
    /* Create thread pool */
    for (int i = 0; i < numberThreads; i++) {
        if(pthread_create(&tid[i], NULL, fnThread, NULL) != 0)
            errorParseCustom("Failed to create thread");
    }

    /* Waiting for all the commands to be executed */
    for (int i = 0; i < numberThreads; i++) {
        if(pthread_join(tid[i], NULL) != 0)
            errorParseCustom("Failed to join thread");
    }

    /* Stop timer */
    stopTimer(&timer);

    print_tecnicofs_tree(fpOut);

    /* release allocated memory */
    destroy_fs();

    printf("TecnicoFS completed in %.4f seconds.\n", timer.elapsedTime);

    /*close previously opened files*/
    fclose(fpRead);
    fclose(fpOut);

    /* release allocated memory */
    free(inputFile);
    free(outputFile);

    pthread_mutex_destroy(&commandsMutex);
    
    exit(EXIT_SUCCESS);
}
