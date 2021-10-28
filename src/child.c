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
#include "utils.h"


/*
	Traverse the directory recursively and search for pattern in files.
	@params:
		name - path to the directory
		pattern - pattern to be recursively searched in the directory

	Note: Feel free to modify the function header if necessary

*/
void dirTraverse(const char *name, char * pattern, ino_t** iNodes, int* iNodesIndex) {	
	DIR *dir = opendir(name);
	struct dirent *entry;

	// Recursively traverse the directory and call SearchForPattern when necessary
	while ((entry = readdir(dir)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

		// Handle hard links
		if (addINodeToListIfUnique(iNodes, MAX_ROOT_SUBDIRS, iNodesIndex, entry->d_ino) == 0) continue;

		char *filePath = (char*)malloc(sizeof(char) * MAX_PATH_LENGTH);
		sprintf(filePath, "%s/%s", name, entry->d_name);

		struct stat* entryStats = (struct stat*) malloc(sizeof(struct stat));
		lstat(filePath, entryStats);
		
		if(!S_ISLNK(entryStats->st_mode)) {			 // Handle symbolic links
			if (entry->d_type == DT_DIR) {
				dirTraverse(filePath, pattern, iNodes, iNodesIndex);
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
		fprintf(stderr,"Child process : %d received %d arguments, expected 3 \n",getpid(), argc);
		fprintf(stderr,"Usage child.o [Directory Path] [Pattern] \n");
		exit(EXIT_FAILURE);
	}

	char* path = argv[1];
	char* pattern = argv[2];
	char buffer[MAX_PATH_LENGTH + 23];  	// "Child received path: %s" has 22 characters + NULL

	sprintf(buffer, "Child received path: %s\n", path);
	write(WRITE_FD, buffer, strlen(buffer));

	ino_t* iNodes = (ino_t*) malloc(sizeof(ino_t) * MAX_ROOT_SUBDIRS);
	int iNodesIndex = 0;

	dirTraverse(path,pattern, &iNodes, &iNodesIndex);
	close(WRITE_FD);

	free(iNodes);

	return(EXIT_SUCCESS);
}
