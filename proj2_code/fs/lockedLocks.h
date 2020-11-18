#ifndef _LOCKEDLOCKS_
#define _LOCKEDLOCKS_

#define INODE_TABLE_SIZE 50

#define READ 0
#define WRITE 1

typedef struct lockedLocks {
	int lockedNodesinumbers[INODE_TABLE_SIZE];
	int numberOfLocks;
    
} LockedLocks;

void lockedLocks_init(LockedLocks *lockedLocks);
void lockedLocks_lock(LockedLocks *lockedLocks, int inumber, int type);
void lockedLocks_addInumber(LockedLocks *lockedLocks, int inumber);
void lockedLocks_tryLock(LockedLocks *lockedLocks, int inumber, int type);
void lockedLocks_unlock(LockedLocks *lockedLocks);

#endif
