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

	if(argc != 3){

		fprintf(stderr,"Usage ./a.out [Path to Directory] [Pattern to search] \n");
		exit(EXIT_FAILURE);
	}
	char* path = argv[1];
	char* pattern = argv[2];

	//Declare any other neccessary variables

	pid_t pid;
	int childProcesses = 0;

	printf("GOT HERE: Variable declaration\n");

	//Open root directory

	DIR *dr = opendir(path);
	printf("GOT HERE: Root directory opened\n");


	// Iterate through root dir and spawn children as neccessary

	struct dirent* entry;
	while ((entry = readdir(dr)) != NULL) {
		printf("Name of entry: %s\n", entry->d_name);
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		char entry_name[100] = {'\0'};

		struct stat* st = (struct stat*)malloc(sizeof(struct stat));
		char *filePath = entry_name;
		stat(filePath, st);

		if (entry->d_type == DT_DIR) {
			// create pipe
			printf("THIS FILE (%s) IS A DIRECTORY.\n", entry->d_name);
			pid = fork();
			if (pid == 0) {
				// Child
				// write to pipe from child process


				if (execl("child", "child", path, pattern, NULL) == -1) {
                    fprintf(stderr, "ERROR: Failed to exec child program.");
                    exit(EXIT_FAILURE);}
			} else {
				childProcesses++; // increment here so root process knows.
				// Parent
				// read from pipe
			}
	} else if (entry->d_type == DT_REG) {
		printf("THIS FILE (%s) IS A REGULAR FILE.\n", entry->d_name);
		//searchPatternInFile(entry, pattern);
	} else {
		printf("THIS FILE IS A DIFFERENT FILE TYPE.\n");
	}
		free(st);
}
	closedir(dr);

		//Process each file/directory in the root dir
		//create a child if the file is directory
			//Create a pipe
			//Do I/o redirection for child
			//Exec child


	printf("Child processes: %d <- should be equal to number of directories\n", childProcesses);

	//Wait for all children to complete
	for (int i = 0; i < childProcesses; i++) {
		wait(NULL);
	}

	//Read pipes of all children and print to stdout
	//Assumption : Pipe never gets full

	return 0;
}
