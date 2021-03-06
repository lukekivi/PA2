/*test machine: csel-broccoli.cselabs.umn.edu
 * group number: G[45]
 * name: Lucas Kivi, Dallas Schauer, Viet Nguyen
 * x500: kivix019, schau364, nguy4471 */

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
				char buffer[MAX_PATH_LENGTH];
				sprintf(buffer, "%s: %s", path, line);
				write(WRITE_FD, buffer, strlen(buffer));
		}
	}

	//Close fp
	fclose(fd_in);
}


// If iNode doesn't exist within the list, add it. Else return a flag value 0.
int addINodeToListIfUnique(ino_t** arr, int size, int* end, ino_t iNode) {
	for (int i = 0; i < *end; i++) {
		if (iNode == ((*arr)[i])) {
			return 0;
		}
	}

	// add iNode to the end of the list and increment the index
	((*arr)[*end]) = iNode;
	*end = *end + 1;
	return 1;
}
