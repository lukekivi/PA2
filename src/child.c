/*test machine: csel-broccoli.cselabs.umn.edu
 * group number: G[45]
 * name: Lucas Kivi, Dallas Schauer, Viet Nguyen
 * x500: kivix019, schau364, nguy4471 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"


/*
	Traverse the directory recursively and search for pattern in files.
	@params:
		name - path to the directory
		pattern - pattern to be recusrively searched in the directory
		iNodes - an array containing a list of iNodes that have been examined in this root subdirectory
		iNodesIndex - current first open index in iNodes
		sizeOfINodes - the size of iNodes

	Note: Feel free to modify the function header if neccessary

*/
void dirTraverse(const char *name, char * pattern, ino_t** iNodes, int* iNodesIndex, int* sizeOfINodes) {	
	DIR *dr;
	if ((dr = opendir(name)) == NULL) {
		fprintf(stderr, "ERROR: Failed to open directory.");
		exit(EXIT_FAILURE);
	}

	struct dirent *entry;
	errno = 0;

	// Recursively traverse the directory and call SearchForPattern when neccessary
	while ((entry = readdir(dr)) != NULL) {
		if (errno != 0) {
			fprintf(stderr, "ERROR: Failed to read directory.");
			exit(EXIT_FAILURE);
		}

		// Avoid self and parent directory
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

		// Check if we the program has already seen this iNode within this directory
		if (addINodeToListIfUnique(iNodes, MAX_ROOT_SUBDIRS, iNodesIndex, entry->d_ino) == 0) continue;
		
		// If iNodes is full, resize it.
		if (*iNodesIndex == *sizeOfINodes) {
			*sizeOfINodes *= 2;
			*iNodes = (ino_t*) realloc(*iNodes, *sizeOfINodes);
		}

		// Build current path
		char *filePath = (char*)malloc(sizeof(char) * MAX_PATH_LENGTH);
		sprintf(filePath, "%s/%s", name, entry->d_name);

		struct stat* entryStats = (struct stat*) malloc(sizeof(struct stat));
		lstat(filePath, entryStats);

		// traverse or search the entry given it isn't a symbolic link
		if(!S_ISLNK(entryStats->st_mode)) {			
			if (entry->d_type == DT_DIR) {
				dirTraverse(filePath, pattern, iNodes, iNodesIndex, sizeOfINodes);
			} else {
				searchPatternInFile(filePath, pattern);
			}
		} 
		
		free(filePath);
		free(entryStats);
	}
}

int main(int argc, char** argv){
	extern int WRITE_FD;

	if(argc !=3){
		fprintf(stderr,"Child process : %d recieved %d arguments, expected 3 \n",getpid(), argc);
		fprintf(stderr,"Usage child.o [Directory Path] [Pattern] \n");
		exit(EXIT_FAILURE);
	}

	char* path = argv[1];
	char* pattern = argv[2];
	char buffer[MAX_PATH_LENGTH + 22];
	
	sprintf(buffer, "Child received path: %s\n", path);
	write(WRITE_FD, buffer, strlen(buffer));

	// Build iNode tracking variables
	int sizeOfInodes = sizeof(ino_t) * MAX_ROOT_SUBDIRS;
	ino_t* iNodes = (ino_t*) malloc(sizeOfInodes);
	int iNodesIndex = 0;

	// dirTraverse does the core of the work
	dirTraverse(path,pattern, &iNodes, &iNodesIndex, &sizeOfInodes);

	close(WRITE_FD);
	free(iNodes);

	return(EXIT_SUCCESS);
}
