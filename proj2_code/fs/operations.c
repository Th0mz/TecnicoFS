#include "operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Given a path, fills pointers with strings for the parent path and child
 * file name
 * Input:
 *  - path: the path to split. ATTENTION: the function may alter this parameter
 *  - parent: reference to a char*, to store parent path
 *  - child: reference to a char*, to store child file name
 */
void split_parent_child_from_path(char * path, char ** parent, char ** child) {

	int n_slashes = 0, last_slash_location = 0;
	int len = strlen(path);

	// deal with trailing slash ( a/x vs a/x/ )
	if (path[len-1] == '/') {
		path[len-1] = '\0';
	}

	for (int i=0; i < len; ++i) {
		if (path[i] == '/' && path[i+1] != '\0') {
			last_slash_location = i;
			n_slashes++;
		}
	}

	if (n_slashes == 0) { // root directory
		*parent = "";
		*child = path;
		return;
	}

	path[last_slash_location] = '\0';
	*parent = path;
	*child = path + last_slash_location + 1;

}


/*
 * Initializes tecnicofs and creates root node.
 */
void init_fs() {
	inode_table_init();
	
	/* create root inode */
	int root = inode_create(T_DIRECTORY);
	
	if (root != FS_ROOT) {
		printf("failed to create node for tecnicofs root\n");
		exit(EXIT_FAILURE);
	}
}


/*
 * Destroy tecnicofs and inode table.
 */
void destroy_fs() {
	inode_table_destroy();
}


/*
 * Checks if content of directory is not empty.
 * Input:
 *  - entries: entries of directory
 * Returns: SUCCESS or FAIL
 */

int is_dir_empty(DirEntry *dirEntries) {
	if (dirEntries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (dirEntries[i].inumber != FREE_INODE) {
			return FAIL;
		}
	}
	return SUCCESS;
}


/*
 * Looks for node in directory entry from name.
 * Input:
 *  - name: path of node
 *  - entries: entries of directory
 * Returns:
 *  - inumber: found node's inumber
 *  - FAIL: if not found
 */
int lookup_sub_node(char *name, DirEntry *entries) {
	if (entries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (entries[i].inumber != FREE_INODE && strcmp(entries[i].name, name) == 0) {
            return entries[i].inumber;
        }
    }
	return FAIL;
}


/*
 * Creates a new node given a path.
 * Input:
 *  - name: path of node
 *  - nodeType: type of node
 * Returns: SUCCESS or FAIL
 */
int create(char *name, type nodeType){

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType;
	union Data pdata;

	LockedLocks lockedLocks;
	lockedLocks_init(&lockedLocks);


	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = lookup_functions(parent_name, &lockedLocks);

	if (parent_inumber == FAIL) {
		printf("failed to create %s, invalid parent dir %s\n",
		        name, parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	lockedLocks_lock(&lockedLocks, parent_inumber, WRITE);
	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		printf("failed to create %s, parent %s is not a dir\n",
		        name, parent_name);
		
		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL) {
		printf("failed to create %s, already exists in dir %s\n",
		       child_name, parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	/* create node and add entry to folder that contains new node */
	child_inumber = inode_create(nodeType);

	lockedLocks_lock(&lockedLocks, child_inumber, WRITE);
	inode_get(child_inumber, &pType, &pdata);

	if (child_inumber == FAIL) {
		printf("failed to create %s in  %s, couldn't allocate inode\n",
		        child_name, parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL) {
		printf("could not add entry %s in dir %s\n",
		       child_name, parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	lockedLocks_unlock(&lockedLocks);

	return SUCCESS;
}


/*
 * Deletes a node given a path.
 * Input:
 *  - name: path of node
 * Returns: SUCCESS or FAIL
 */
int delete(char *name){

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType, cType;
	union Data pdata, cdata;

	LockedLocks lockedLocks;
	lockedLocks_init(&lockedLocks);

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = lookup_functions(parent_name, &lockedLocks);

	if (parent_inumber == FAIL) {
		printf("failed to delete %s, invalid parent dir %s\n",
		        child_name, parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	lockedLocks_lock(&lockedLocks, parent_inumber, WRITE);
	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		printf("failed to delete %s, parent %s is not a dir\n",
		        child_name, parent_name);
		
		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);

	if (child_inumber == FAIL) {
		printf("could not delete %s, does not exist in dir %s\n",
		       name, parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	lockedLocks_lock(&lockedLocks, child_inumber, WRITE);
	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL) {
		printf("could not delete %s: is a directory and not empty\n",
		       name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL) {
		printf("failed to delete %s from dir %s\n",
		       child_name, parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL) {
		printf("could not delete inode number %d from dir %s\n",
		       child_inumber, parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	lockedLocks_unlock(&lockedLocks);

	return SUCCESS;
}

/* Removes the inode of the file to move from the origin directory */
int delete_entry(char *name, DirEntry *entries) {
	if (entries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (entries[i].inumber != FREE_INODE && strcmp(entries[i].name, name) == 0) {
            /* Remove entry from dirEntry */

			entries[i].inumber = FREE_INODE;
			entries[i].name[0] = '\0';

			return SUCCESS;
        }
    }

	return FAIL;
}

/* Puts the inode of the file to move in the destination directory */
int insert_entry(char *name, int inumber, DirEntry *entries) {
	if (entries == NULL) {
		return FAIL;
	}

	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (entries[i].inumber == FREE_INODE) {
            /* Remove entry from dirEntry */

			entries[i].inumber = inumber;
			strcpy(entries[i].name, name);

			return SUCCESS;
        }
    }

	return FAIL;
}

/* Moves the given file in the origin path to his new path destination if possible */
int move(char *origin, char *destination) {

	int origin_parent_inumber, origin_child_inumber; 
	int destination_parent_inumber, destination_child_inumber;

	char *origin_parent_name, *origin_child_name; 
	char origin_name_copy[MAX_FILE_NAME];

	char *destination_parent_name, *destination_child_name;
	char destination_name_copy[MAX_FILE_NAME];

	/* use for copy */
	type originPType, originCType, destinationPType;
	union Data originPData, originCData, destinationPData;

	LockedLocks lockedLocks;
	lockedLocks_init(&lockedLocks);

	strcpy(origin_name_copy, origin);
	split_parent_child_from_path(origin_name_copy, &origin_parent_name, &origin_child_name);


	origin_parent_inumber = lookup_functions(origin_parent_name, &lockedLocks);

	/* Test : origin path exists */
		/* Check if parent is valid */
	if (origin_parent_inumber == FAIL) {
		printf("failed to move %s, invalid origin dir %s\n",
		        origin_child_name, origin_parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	lockedLocks_lock(&lockedLocks, origin_parent_inumber, WRITE);
	inode_get(origin_parent_inumber, &originPType, &originPData);

	if(originPType != T_DIRECTORY) {
		printf("failed to move %s, parent %s is not a dir\n",
		        origin_child_name, origin_parent_name);
		
		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	/* Check if child is valid */
	origin_child_inumber = lookup_sub_node(origin_child_name, originPData.dirEntries);

	if (origin_child_inumber == FAIL) {
		printf("could not move %s, does not exist in origin dir %s\n",
		       origin, origin_parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	lockedLocks_lock(&lockedLocks, origin_child_inumber, WRITE);
	inode_get(origin_child_inumber, &originCType, &originCData);

	if (originCType == T_NONE) {
		printf("could not move %s: not a directory neither a file\n",
		       origin);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	strcpy(destination_name_copy, destination);
	split_parent_child_from_path(destination_name_copy, &destination_parent_name, &destination_child_name);

	destination_parent_inumber = lookup_functions(destination_parent_name, &lockedLocks);

	
	/* Test : destination path exists */
	/* Check if parent is valid	*/
	/* If case to see if the move is made only to change the name of the file*/
	if (origin_parent_inumber != destination_parent_inumber) {
		if (destination_parent_inumber == FAIL) {
			printf("failed to move %s, invalid parent dir %s\n",
		    destination_child_name, destination_parent_name);

			lockedLocks_unlock(&lockedLocks);
			return FAIL;
		}

		lockedLocks_lock(&lockedLocks, destination_parent_inumber, WRITE);
		inode_get(destination_parent_inumber, &destinationPType, &destinationPData);

		if(destinationPType != T_DIRECTORY) {
			printf("failed to move %s, destination parent %s is not a dir\n",
					destination_child_name, destination_parent_name);
			
			lockedLocks_unlock(&lockedLocks);
			return FAIL;
		}
	} else {
		destinationPData = originPData;
	}


	/* Check if child is valid */
	destination_child_inumber = lookup_sub_node(destination_child_name, destinationPData.dirEntries);

	/* Checks if there is a file with the same name in destiny directory */
	if (destination_child_inumber != FAIL) {
		printf("could not move %s, file with same name exists in dir %s\n",
		       destination, destination_parent_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
	}

	if (delete_entry(origin_child_name, originPData.dirEntries) == FAIL) {
		printf("Cant remove %s from directory\n", origin_child_name);

		lockedLocks_unlock(&lockedLocks);
		return FAIL;
		
	}

	if (insert_entry(destination_child_name, origin_child_inumber, destinationPData.dirEntries) == FAIL) {
		printf("Cant insert %s in directory\n", origin_child_name);

		
		lockedLocks_unlock(&lockedLocks);
		return FAIL;

	}

	/*INTERBLOCAGEM CODIGO MEGA MERDOSO

	while(1) {
		int canMove = 0;
		while(!canMove) {
			//mudar os locks do insert_delay para se poder fazer aqui as verificaçoes yeeeeet
		}
	}
*/
	lockedLocks_unlock(&lockedLocks);
	return SUCCESS;
}


/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */
int lookup(char *name) {
	LockedLocks lockedLocks;
	lockedLocks_init(&lockedLocks);

	char *saveptr;
	char full_path[MAX_FILE_NAME];
	char delim[] = "/";

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	lockedLocks_lock(&lockedLocks, current_inumber, READ);
	/* get root inode data */
	inode_get(current_inumber, &nType, &data);

	char *path = strtok_r(full_path, delim, &saveptr);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		lockedLocks_lock(&lockedLocks, current_inumber, READ);

		inode_get(current_inumber, &nType, &data);
		path = strtok_r(NULL, delim, &saveptr); 
	}

	lockedLocks_unlock(&lockedLocks);

	return current_inumber;
}

/**
 *  Similar to lookup but it doesnt lock the node we are looking for
 */
int lookup_functions(char *name, LockedLocks *lockedLocks) {

	char *saveptr;
	char full_path[MAX_FILE_NAME];
	char delim[] = "/";

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	/* Checks if the full path as some thing in it	*/
	if (strcmp(full_path, "") != 0) {
		lockedLocks_lock(lockedLocks, current_inumber, READ);
	}
	
	/* get root inode data */
	inode_get(current_inumber, &nType, &data);

	char *path = strtok_r(full_path, delim, &saveptr);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		
		path = strtok_r(NULL, delim, &saveptr);
		if (path != NULL) {
			lockedLocks_lock(lockedLocks, current_inumber, READ);
			inode_get(current_inumber, &nType, &data);
		}
	}

	return current_inumber;
}

/*
 * Prints tecnicofs tree.
 * Input:
 *  - fp: pointer to output file
 */
void print_tecnicofs_tree(FILE *fp){
	inode_print_tree(fp, FS_ROOT, "");
}
