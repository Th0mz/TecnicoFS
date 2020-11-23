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
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "fs/operations.h"
#include <errno.h>

#include "timer.h"
#include <pthread.h>

#define FALSE 0
#define TRUE 1

#define MAX_INPUT_SIZE 100


/*Global mutex to control the synchronization of the access to the vector of commands */
pthread_mutex_t commandsMutex = PTHREAD_MUTEX_INITIALIZER;

/* Number of consumer threds */
int numberThreads = 0;

int sockfd, addressLen;
struct sockaddr_un server_address;


void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

/* Similar to "errorParse" but prints custom messages */
void errorParseCustom(char *error) {
    fprintf(stderr, "Error: %s\n", error);
    exit(EXIT_FAILURE);
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

void *processMessages(void *arg) {
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

int setSocketAddrUn(char *path, struct sockaddr_un *address) {
    
    if (address == NULL) 
        return 0;
    
    bzero((char *) address, sizeof(struct sockaddr_un));
    address->sun_family = AF_UNIX;
    strcpy(address->sun_path, path);
    
    return SUN_LEN(address);
}

void initSocket(char *socketName) {
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        errorParseCustom("failed to create socket");
    }

    unlink(socketName);    
    addressLen = setSocketAddrUn(socketName, &server_address);
    if (bind(sockfd, (struct sockaddr *) &server_address, addressLen) < 0) {
        printf("errno : %d", errno);
        errorParseCustom("failed to bind name to the socket");
    }
}

/* Frees all allocated memory */
void closeProgram() {
    /* release allocated memory */
    destroy_fs();

    pthread_mutex_destroy(&commandsMutex);
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    Timer timer;

    /* init filesystem */
    init_fs();
    
    /* Error : Invalid number of arguments */
    if (argc != 3) {
        errorParseCustom("Invalid number of arguments");
    }

    numberThreads = atoi(argv[1]);
    /* Error : numberThreads not an int or <= 0 */
    if (numberThreads <= 0) {
        errorParseCustom("numberThreads not an int or <= 0");
    }

    char *socketName = argv[2];
    initSocket(socketName);

    /* Create thread pool and execute commands */
    pthread_t tid[numberThreads];

    /* Start timer*/
    startTimer(&timer);
    
    for (int i = 0; i < numberThreads; i++) {
        if(pthread_create(&tid[i], NULL, processMessages, NULL) != 0)
            errorParseCustom("Failed to create thread");
    }


    for (int i = 0; i < numberThreads; i++) {
        if(pthread_join(tid[i], NULL) != 0)
            errorParseCustom("Failed to join thread");
    }

    /* Stop timer */
    stopTimer(&timer);

    printf("TecnicoFS completed in %.4f seconds.\n", timer.elapsedTime);
    closeProgram();
}
