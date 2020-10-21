#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "lock_functions.h"

/* Global lock variables */
int syncStrategy;
void *globalLock;

void errorParseLock(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

/*Defines the synchronization strategy for threading
  Inputs:
     - syncMode: argument for stdin that declares the synchronization strategy to be adopted
*/
void chooseSync(char *syncMode, int numberThreads) {
        /* Define sync mode*/
    if (strcmp(syncMode, "mutex") == 0) {
        syncStrategy = MUTEX; 
        lock_init(syncStrategy);
    } else if (strcmp(syncMode, "rwlock") == 0){
        syncStrategy = RWLOCK;
        lock_init(syncStrategy);
    } else if (strcmp(syncMode, "nosync") == 0){
        /* Perguntar ao stor se é para por esse coisa */
        if (numberThreads != 1)
            errorParseLock();
        
        syncStrategy = NOSYNC;
    } else
        errorParseLock();
}


/*Allocates memory for globalLock and casts it to the lockof the synchronization strategy requested
  Input:
     - syncStrategy: integer that represents the synchronization strategy adopted
*/
void lock_init(int syncStrategy) {
    if (syncStrategy == MUTEX) {
        globalLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
        if(pthread_mutex_init(globalLock, NULL) != 0)
            errorParseLock();
    }

    else if (syncStrategy == RWLOCK) {
        globalLock = (pthread_rwlock_t*) malloc(sizeof(pthread_rwlock_t));
        if(pthread_rwlock_init(globalLock, NULL) != 0)
            errorParseLock();
    }
}

/*Locks the global lock with the type of lock that is adopted by the synchronizations strategy
  Input:
     - syncStrategy: integer that represents the synchronization strategy adopted
*/
void lock(int syncStrategy, int rwLockType) {
    if (syncStrategy == MUTEX) {
        if(pthread_mutex_lock(globalLock) != 0) 
            errorParseLock();
    }
    /*If its a read-write lock it also needs to know if it locks for only reading, or writing also*/

    else if (syncStrategy == RWLOCK && rwLockType == RDLOCK) {
        if(pthread_rwlock_rdlock(globalLock) != 0)
            errorParseLock();
    }
    else if (syncStrategy == RWLOCK && rwLockType == WRLOCK) {
        if(pthread_rwlock_wrlock(globalLock) != 0)
            errorParseLock();
    }
}


/*Unlocks the global lock with the type of lock that is adopted by the synchronizations strategy
  Input:
     - syncStrategy: integer that represents the synchronization strategy adopted
*/
void unlock(int syncStrategy) {
    if (syncStrategy == MUTEX) {
        if(pthread_mutex_unlock(globalLock) != 0)
            errorParseLock();
    }
    else if (syncStrategy == RWLOCK) {
        if(pthread_rwlock_unlock(globalLock) != 0)
            errorParseLock();
    }
}

/*Destroys global lock
  Input:
     - lock: pointer for the global lock
*/
void destroy_allLocks(pthread_mutex_t commandsMutex, int syncStrategy) {
    if (syncStrategy == MUTEX) {
        if (pthread_mutex_destroy(globalLock) != 0 && 
            pthread_mutex_destroy(&commandsMutex) != 0)
                errorParseLock();
    } else if (syncStrategy == RWLOCK) {
        if(pthread_rwlock_destroy(globalLock) != 0 && 
           pthread_mutex_destroy(&commandsMutex) != 0)
                errorParseLock();
    }
    
    free(globalLock);
}




