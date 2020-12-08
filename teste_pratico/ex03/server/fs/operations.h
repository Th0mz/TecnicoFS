#ifndef FS_H
#define FS_H
#include "state.h"
#include "lockedLocks.h"

void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType);
int delete(char *name);
int move(char *origin, char *destination);
int lookup(char *name);
int lookup_functions(char *name, LockedLocks *lockedLocks);
int lookup_move(char *name, LockedLocks *lockedLocks);
void print_tecnicofs_tree(FILE *fp);
int printTree(char *outputFile);

#endif /* FS_H */
