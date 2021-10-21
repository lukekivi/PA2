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
	int fds[2];

	// Iterate through root dir and spawn children as neccessary
	struct dirent* entry;
	while ((entry = readdir(dr)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

		char *filePath = (char*) malloc(sizeof(char) * STRING_BUFFER);
		sprintf(filePath, "%s/%s", path, entry->d_name);

		if (entry->d_type == DT_DIR) {
			// create pipe
			if (pipe(fds) < 0) {
		        fprintf(stderr, "ERROR: Failed to open pipe\n");
		        exit(EXIT_FAILURE);
    		}

			if ((pid = fork()) == -1) {
				fprintf(stderr, "ERROR: Failed to fork\n");
		        exit(EXIT_FAILURE);
			} else if (pid == 0) {
				// Child
				// write to pipe from child process
				printf("%d\n", WRITE_FD);
				if (dup2(fds[1], WRITE_FD) == -1) {
					fprintf(stderr, "ERROR: Failed to exec child program\n.");
                    exit(EXIT_FAILURE);
				}
				close(fds[0]);
				close(fds[1]);

				if (execl("child", "child", filePath, pattern, NULL) == -1) {
                    fprintf(stderr, "ERROR: Failed to exec child program\n.");
                    exit(EXIT_FAILURE);
				}		
			} else {
				childProcesses++; // increment here so root process knows.
				// Parent
				// read from pipe
				close(fds[1]);
				char* rcv_buffer = (char *) malloc(sizeof(char) * STRING_BUFFER);
				
        		if (read(fds[0], rcv_buffer, dataSize) < 0) {
            		fprintf(stderr, "ERROR: Failed to read\n");
        		} else {
					printf("Child info from parent: %s", rcv_buffer);
				}

				// done reading
				close(fds[1]);
				free(rcv_buffer);
			} 
	} else if (entry->d_type == DT_REG) {
		printf("THIS FILE (%s) IS A REGULAR FILE.\n", entry->d_name);
		//searchPatternInFile(entry, pattern);
	} else {
		printf("THIS FILE IS A DIFFERENT FILE TYPE.\n");
	}
	free(filePath);
}
	closedir(dr);

		//Process each file/directory in the root dir
		//create a child if the file is directory
			//Create a pipe
			//Do I/o redirection for child
			//Exec child

	printf("Child processes: %d\n", childProcesses);

	//Wait for all children to complete
	for (int i = 0; i < childProcesses; i++) {
		wait(NULL);
	}

	//Read pipes of all children and print to stdout
	//Assumption : Pipe never gets full

	return 0;
}