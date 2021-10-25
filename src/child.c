#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include<string.h>
#include<stdlib.h>
#include "utils.h"


/*
	Traverse the directory recursively and search for pattern in files.
	@params:
		name - path to the directory
		pattern - pattern to be recusrively searched in the directory

	Note: Feel free to modify the function header if neccessary

*/
void dirTraverse(const char *name, char * pattern)
{
	extern int STRING_BUFFER;
	DIR *dir = opendir(name);
	struct dirent *entry;

	// Recursively traverse the directory and call SearchForPattern when neccessary
	while ((entry = readdir(dir)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

		char *filePath = (char*)malloc(sizeof(char) * STRING_BUFFER);
		sprintf(filePath, "%s/%s", name, entry->d_name);

		struct stat* entryStats = (struct stat*) malloc(sizeof(struct stat));
		lstat(filePath, entryStats);

		if(!S_ISLNK(entryStats->st_mode)) {			
			if (entry->d_type == DT_DIR) {
					dirTraverse(filePath, pattern);
			} else {
				searchPatternInFile(filePath, pattern);
			}
		} else {
			// This means a symbolic link was found
			fprintf(stdout, "%s was a symbolic link", filePath);
		}
		free(filePath);
		free(entryStats);
	}
}

int main(int argc, char** argv){
	extern int STRING_BUFFER;
	extern int WRITE_FD;

	int dataSize = sizeof(char) * STRING_BUFFER;

	if(argc !=3){
		fprintf(stderr,"Child process : %d recieved %d arguments, expected 3 \n",getpid(), argc);
		fprintf(stderr,"Usage child.o [Directory Path] [Pattern] \n");
		exit(EXIT_FAILURE);
	}

	char* path = argv[1];
	char* pattern = argv[2];
	char buffer[STRING_BUFFER];
	
	sprintf(buffer, "Child process: %d received path: %s\n", getpid(), path);

	write(WRITE_FD, buffer, sizeof(buffer));

	dirTraverse(path,pattern);
	close(WRITE_FD);

	return(EXIT_SUCCESS);
}
