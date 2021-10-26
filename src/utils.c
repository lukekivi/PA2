#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"

const int WRITE_FD = STDOUT_FILENO;

/*
	Searches for pattern in file and writes the entire line containing the pattern to STDOUT
	Format for print: [path to file] : Line in the file
	@Params:
		path - path to the file
		pattern : pattern to be searched
*/
void searchPatternInFile(char* path, char* pattern) {

	FILE * fd_in;
	
	if ((fd_in = fopen(path, "r")) == NULL) {
		fprintf(stderr,"Error opening file: %s\n",path);
		return;
	}

	size_t stringBuffer = MAX_PATH_LENGTH*2;

	char * line = NULL;
	char * subLine = NULL;
	int nbytes;

	//Read file line by line and check if pattern exists in the line
	while (nbytes = getline(&line, &stringBuffer, fd_in) > 0) {
		if((subLine = strstr(line, pattern)) != NULL) {
			if (*(subLine-1) == ' ' || (strcmp(subLine, line) == 0)) { //make sure the substring is actually a word and not just part of a word.
					char buffer[MAX_PATH_LENGTH];
					sprintf(buffer, "%s: %s", path, line);
					write(WRITE_FD, buffer, strlen(buffer));				
					fprintf(stderr, "%s", buffer);
			}
		}
	}

	//Close fp
	fclose(fd_in);
}


int addINodeToListIfUnique(ino_t arr[], int size, int end, ino_t iNode) {
	for (int i = 0; i < end+1; i++) {
		if (iNode == arr[i]) {
			return 0;
		}
	}
	arr[end+1] = iNode;
	return 1;
}
