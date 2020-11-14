#include <stdio.h>
#include <pthread.h>

#include "state.h"
#include "lockedLocks.h"

/* Inits the lockedLockes
*/
void lockedLocks_init(LockedLocks *lockedLocks) {
	lockedLocks->numberOfLocks = 0;
}

/* Given a lock and a type, locks the lock according to the 
* type passed and adds that lock to a list of all the locks
*  used in that funcion 
*/
void lockedLocks_lock(LockedLocks *lockedLocks, int inumber , int type) {

    pthread_rwlock_t *lock = inode_getLock(inumber);
    
    if (lock == NULL) {
        return;
    }

	if(type == READ) {
		if(pthread_rwlock_rdlock(lock) != 0) { 
			exit(EXIT_FAILURE); 
		}

		// DEBUG: printf("[Lock] inumber : %d -> READ\n", inumber);
	}
	else if (type == WRITE){ 
		if(pthread_rwlock_wrlock(lock) != 0) { 
			exit(EXIT_FAILURE);
		}

		// DEBUG: printf("[Lock] inumber : %d -> WRITE\n", inumber);	
	}

	int position = lockedLocks->numberOfLocks;
	lockedLocks->lockedNodesinumbers[position] = inumber;

	lockedLocks->numberOfLocks++;	
}

/* Given all the lockedLocks unlocks them all 
*/
void lockedLocks_unlock(LockedLocks *lockedLocks) {
	int numberOfLocks = lockedLocks->numberOfLocks; 
    
	for (int i = 0; i < numberOfLocks; i++) {
		pthread_rwlock_t *lock = inode_getLock(lockedLocks->lockedNodesinumbers[i]);
        
		if (lock == NULL) {
            return;
        }
        
        if(pthread_rwlock_unlock(lock) != 0) {
			exit(EXIT_FAILURE);
		}

		// DEBUG: printf("[Unlock] inumber : %d\n", lockedLocks->lockedNodesinumbers[i]);	
	}
	
}