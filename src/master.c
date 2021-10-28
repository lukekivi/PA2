#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"


int main(int argc, char** argv){
	extern int WRITE_FD;
	
	if(argc != 3){
		fprintf(stderr,"Usage ./a.out [Path to Directory] [Pattern to search] \n");
		exit(EXIT_FAILURE);
	}
	char* path = argv[1];
	char* pattern = argv[2];

	//Open root directory
	DIR *dr;
	if ((dr = opendir(path)) == NULL) {
		fprintf(stderr, "ERROR: Failed to open directory.");
		exit(EXIT_FAILURE);
	}

	
	//Declare any other neccessary variables
	pid_t pid;
	int childProcesses = 0;

	int fds[MAX_ROOT_SUBDIRS][2];

	ino_t* iNodes = (ino_t*) malloc(sizeof(ino_t) * MAX_ROOT_SUBDIRS);
	int iNodesIndex = 0;

	// Iterate through root dir and spawn children as neccessary
	struct dirent* entry;
	errno = 0;
	while ((entry = readdir(dr)) != NULL) {
		if (errno != 0) {
			fprintf(stderr, "ERROR: Failed to read from directory.");
			exit(EXIT_FAILURE);
		}

		// Avoid self and parent directory
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

		// Check if we the program has already seen this iNode within this directory
		if (addINodeToListIfUnique(&iNodes, MAX_ROOT_SUBDIRS, &iNodesIndex, entry->d_ino) == 0) continue;

		// Build current path
		char *filePath = (char*) malloc(sizeof(char) * MAX_PATH_LENGTH);
		sprintf(filePath, "%s/%s", path, entry->d_name);

		struct stat* entryStats = (struct stat*) malloc(sizeof(struct stat));
		lstat(filePath, entryStats);
		
		//Process each file/directory in the root dir given it isn't a symbolic link
		if(!S_ISLNK(entryStats->st_mode)) {

			//create a child if the file is a directory
			if (entry->d_type == DT_DIR) {

				// create pipe
				if (pipe(fds[childProcesses]) < 0) {
					fprintf(stderr, "ERROR: Failed to open pipe\n");
					exit(EXIT_FAILURE);
				}

				if ((pid = fork()) == -1) {
					fprintf(stderr, "ERROR: Failed to fork\n");
					exit(EXIT_FAILURE);
				} else if (pid == 0) {

					//Do I/o redirection for child
					if (dup2(fds[childProcesses][1], WRITE_FD) == -1) {
						fprintf(stderr, "ERROR: Failed to exec child program\n.");
						exit(EXIT_FAILURE);
					}

					close(fds[childProcesses][0]);
					close(fds[childProcesses][1]);

					//Exec child
					if (execl("child", "child", filePath, pattern, NULL) == -1) {
						fprintf(stderr, "ERROR: Failed to exec child program\n.");
						exit(EXIT_FAILURE);
					}		
				} else {
					// Parent
					// read from pipe
					close(fds[childProcesses][1]);
				} 
			childProcesses++; // increment so main process has a child count
		} else {
			// This is a non-directory and non-symbolic link file
			searchPatternInFile(filePath, pattern);
		} 
	}
	
	free(entryStats);
	free(filePath);
}
	closedir(dr);

	//Wait for all children to complete
	for (int i = 0; i < childProcesses; i++) {
		wait(NULL);
	}

	size_t stringBuffer = MAX_PATH_LENGTH * 2;
	char* rcv_buffer = NULL;
	
	//Read pipes of all children and print to stdout
	//Assumption : Pipe never gets full
	for (int i = 0; i < childProcesses; i++) {
		FILE* fp;
		if ((fp = fdopen(fds[i][0], "r")) == NULL) {
			fprintf(stderr, "ERROR: Failed to open file descriptor");
		}
		while (getline(&rcv_buffer, &stringBuffer, fp) > 0) {	
				printf("%s", rcv_buffer);
		}
		// done reading
		close(fds[i][0]);
		fclose(fp);
	}

	free(rcv_buffer);
	free(iNodes);

	return 0;
}