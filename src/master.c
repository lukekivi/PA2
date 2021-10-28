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
	extern int WRITE_FD;

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
	if (dr == NULL) {
		fprintf(stderr,"Could not open file. \n");
		exit(EXIT_FAILURE);
	}
	int fds[MAX_ROOT_SUBDIRS][2];
	int numberOfSubDirs = 0;

	ino_t* iNodes = (ino_t*) malloc(sizeof(ino_t) * MAX_ROOT_SUBDIRS);
	int iNodesIndex = 0;

	// Iterate through root dir and spawn children as neccessary
	struct dirent* entry;
	while ((entry = readdir(dr)) != NULL) {
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		if (addINodeToListIfUnique(&iNodes, MAX_ROOT_SUBDIRS, &iNodesIndex, entry->d_ino) == 0) continue;

		char *filePath = (char*) malloc(sizeof(char) * MAX_PATH_LENGTH);
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
	for (int i = 0; i < numberOfSubDirs; i++) {
		FILE* fp = fdopen(fds[i][0], "r");
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
