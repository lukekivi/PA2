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
		pattern - pattern to be recusrively searched in the directory

	Note: Feel free to modify the function header if neccessary

*/
void dirTraverse(const char *name, char * pattern, ino_t** iNodes, int* iNodesIndex) {	
	DIR *dir = opendir(name);
	struct dirent *entry;

	// Recursively traverse the directory and call SearchForPattern when neccessary
	while ((entry = readdir(dir)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		if (addINodeToListIfUnique(iNodes, MAX_ROOT_SUBDIRS, iNodesIndex, entry->d_ino) == 0) continue;

		char *filePath = (char*)malloc(sizeof(char) * MAX_PATH_LENGTH);
		sprintf(filePath, "%s/%s", name, entry->d_name);

		struct stat* entryStats = (struct stat*) malloc(sizeof(struct stat));
		lstat(filePath, entryStats);

		if(!S_ISLNK(entryStats->st_mode)) {			
			if (entry->d_type == DT_DIR) {
				dirTraverse(filePath, pattern, iNodes, iNodesIndex);
			} else {
				searchPatternInFile(filePath, pattern);
			}
		} else {
			// This means a symbolic link was found
			char buffer[MAX_PATH_LENGTH];
			sprintf(buffer, "%s was a symbolic link", filePath);
			write(STDOUT_FILENO, buffer, strlen(buffer));
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
	char buffer[MAX_PATH_LENGTH];
	
	sprintf(buffer, "Child process: %d received path: %s\n", getpid(), path);
	write(WRITE_FD, buffer, strlen(buffer));

	ino_t* iNodes = (ino_t*) malloc(sizeof(ino_t) * MAX_ROOT_SUBDIRS);
	int iNodesIndex = 0;

	dirTraverse(path,pattern, &iNodes, &iNodesIndex);
	close(WRITE_FD);

	free(iNodes);

	return(EXIT_SUCCESS);
}
