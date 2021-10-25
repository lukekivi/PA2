#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"


int main(int argc, char** argv){
	extern int STRING_BUFFER;
	extern int WRITE_FD;
	const int MAX_NUMBER_SUB_DIRS = 10;
	const int PIPE_READ_SIZE = 10000;

	int dataSize = sizeof(char) * STRING_BUFFER;

	if(argc != 3){
		fprintf(stderr,"Usage ./a.out [Path to Directory] [Pattern to search] \n");
		exit(EXIT_FAILURE);
	}
	char* path = argv[1];
	char* pattern = argv[2];

	//Declare any other neccessary variables

	pid_t pid;
	int childProcesses = 0;

	//Open root directory

	DIR *dr = opendir(path);
	int fds[MAX_NUMBER_SUB_DIRS][2];
	int numberOfSubDirs = 0;


	// Iterate through root dir and spawn children as neccessary
	struct dirent* entry;
	while ((entry = readdir(dr)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		
		char *filePath = (char*) malloc(sizeof(char) * STRING_BUFFER);
		sprintf(filePath, "%s/%s", path, entry->d_name);

		struct stat* entryStats = (struct stat*) malloc(sizeof(struct stat));
		lstat(filePath, entryStats);
		
		//Process each file/directory in the root dir
		if(!S_ISLNK(entryStats->st_mode)) {

			//create a child if the file is directory
			if (entry->d_type == DT_DIR) {

				// create pipe
				if (pipe(fds[numberOfSubDirs]) < 0) {
					fprintf(stderr, "ERROR: Failed to open pipe\n");
					exit(EXIT_FAILURE);
				}

				if ((pid = fork()) == -1) {
					fprintf(stderr, "ERROR: Failed to fork\n");
					exit(EXIT_FAILURE);
				} else if (pid == 0) {

					//Do I/o redirection for child
					if (dup2(fds[numberOfSubDirs][1], WRITE_FD) == -1) {
						fprintf(stderr, "ERROR: Failed to exec child program\n.");
						exit(EXIT_FAILURE);
					}
					close(fds[numberOfSubDirs][0]);
					close(fds[numberOfSubDirs][1]);

					//Exec child
					if (execl("child", "child", filePath, pattern, NULL) == -1) {
						fprintf(stderr, "ERROR: Failed to exec child program\n.");
						exit(EXIT_FAILURE);
					}		
				} else {
					childProcesses++; // increment here so root process knows.
					// Parent
					// read from pipe
					close(fds[numberOfSubDirs][1]);
				} 
			numberOfSubDirs += 1;
		} else {
			// This is a non-directory and non-symbolic link file
			searchPatternInFile(filePath, pattern);
		} 
	} else {
		// This means a symbolic link was found
		printf("%s was a symbolic link\n", filePath);
	}
	free(entryStats);
	free(filePath);
}
	closedir(dr);

	//Wait for all children to complete
	for (int i = 0; i < childProcesses; i++) {
		wait(NULL);
	}

	long int buffSize = sizeof(char) * PIPE_READ_SIZE;
	char* rcv_buffer = (char*) malloc(sizeof(char) * buffSize);
	//Read pipes of all children and print to stdout
	//Assumption : Pipe never gets full
	for (int i = 0; i < numberOfSubDirs; i++) {
		while (read(fds[i][0], rcv_buffer, buffSize) > 0) {	
				printf("%s", rcv_buffer);
		}
		// done reading
		close(fds[i][0]);
	}

	free(rcv_buffer);

	return 0;
}