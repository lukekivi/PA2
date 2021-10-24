#include<stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>
#include "utils.h"

const int STRING_BUFFER = 100;
const int WRITE_FD = STDOUT_FILENO;

/*
	Searches for pattern in file and writes the entire line containing the pattern to STDOUT
	Format for print: [path to file] : Line in the file
	@Params:
		path - path to the file
		pattern : pattern to be searched
*/
void searchPatternInFile(char* path, char* pattern){


	FILE * fp;

	fp = fopen(path, "r");
	if (fp == NULL){
		fprintf(stderr,"Error opening file: %s \n",path);
		return;
	}

	char buffer[STRING_BUFFER];
	char * line = (char*)malloc(sizeof(char)*STRING_BUFFER);
	char *filePath = (char*) malloc(sizeof(char) * STRING_BUFFER);

	while (fgets(buffer, STRING_BUFFER, fp) != NULL) {
		if((line = strstr(buffer, pattern)) != NULL) {
				if (*(line-1) == ' ' || (strcmp(buffer, line) == 0)) //make sure the substring is actually a word and not just part of a word.
				{
						printf("%s: %s\n", path, buffer);
				}
		}
	}

	//Read file line by line and check if pattern exists in the line

	//Close fp
	//free(line);                // these frees cause an error for some reason in run3?
	//free(filePath);
	fclose(fp);
}
