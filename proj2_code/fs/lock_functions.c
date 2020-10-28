#include "lock_functions.h"
#include "state.h"

void lockedLocks_init(LockedLocks *lockedLocks) {
	lockedLocks->numberOfLocks = 0;
}

/* Given a lock and a type, locks the lock according to the 
* type passed and adds that lock to a list of all the locks
*  used in that funcion 
*/
void lockedLocks_lock(int inumber, LockedLocks *lockedLocks, int type) {
    pthread_rwlock_t lock;
    inode_getLock(inumber, &lock);

	if(type == READ) {
		if(pthread_rwlock_rdlock(&lock) != 0) { 
			exit(EXIT_FAILURE); 
		}
	}
	else if (type == WRITE){ 
		if(pthread_rwlock_wrlock(&lock) != 0) { 
			exit(EXIT_FAILURE);
		}	
	}

	int position = lockedLocks->numberOfLocks;
	lockedLocks->inumbers[position] = inumber;

	lockedLocks->numberOfLocks++;	
}

/* Given all the lockedLocks unlocks them all 
*/
void lockedLocks_unlock(LockedLocks *lockedLocks) {
	int numberOfLocks = lockedLocks->numberOfLocks;
    int inumber;
    pthread_rwlock_t lock;
	
	for (int i = 0; i < numberOfLocks; i++) {
		inumber = lockedLocks->inumbers[i];
        inode_getLock(inumber, &lock);

        if(pthread_rwlock_unlock(&lock) != 0) {
			exit(EXIT_FAILURE);
		}
	}
	
}
