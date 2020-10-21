#ifndef _LOCK_FUNCTIONS_
#define _LOCK_FUNCTIONS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define NOSYNC 0
#define MUTEX 1
#define RWLOCK 2

#define RDLOCK 3
#define WRLOCK 4

/* Extern lock variables*/
extern int syncStrategy;
extern void *globalLock;

void chooseSync(char *syncMode, int numberThreads);
void lock_init(int syncStrategy);
void lock(int syncStrategy, int rwLockType);
void unlock(int syncStrategy);
void destroy_allLocks(pthread_mutex_t commandsMutex, int syncStrategy);


#endif /*_LOCK_FUNCTIONS_*/