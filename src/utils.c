#include<stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>
#include "utils.h"

const int STRING_BUFFER = 1001;
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

	char buffer[STRING_BUFFER];
	char * line = NULL;
	//Read file line by line and check if pattern exists in the line
	while (fgets(buffer, STRING_BUFFER, fd_in) != NULL) {
		if((line = strstr(buffer, pattern)) != NULL) {
				if (*(line-1) == ' ' || (strcmp(buffer, line) == 0)) //make sure the substring is actually a word and not just part of a word.
				{
					fprintf(stdout, "%s: %s", path, buffer);
				}
		}
	}

	//Close fp
	fclose(fd_in);
}
