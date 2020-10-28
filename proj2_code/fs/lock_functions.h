#ifndef _LOCK_FUNCTIONS_
#define _LOCK_FUNCTIONS_

#include "state.h"

#define READ 0
#define WRITE 1

typedef struct lockedLocks {
	int inumbers[INODE_TABLE_SIZE];
	int numberOfLocks;
} LockedLocks;


void lockedLocks_init(LockedLocks *lockedLocks);
void lockedLocks_lock(int inumber, LockedLocks *lockedLocks, int type);
void lockedLocks_unlock(LockedLocks *lockedLocks);

#endif