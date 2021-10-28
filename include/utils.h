#ifndef UTILS_H
#define UTILS_H

/*test machine: csel-broccoli.cselabs.umn.edu
 * group number: G[45]
 * name: Lucas Kivi, Dallas Schauer, Viet Nguyen
 * x500: kivix019, schau364, nguy4471 */

#include <dirent.h>
#define MAX_ROOT_SUBDIRS 100
#define MAX_READ_LENGTH 5000
#define MAX_PATH_LENGTH 1024
#define MAX_WRITE_LENGTH 5000 
#define MAX_INODE_LENGTH 128

/*
	Search for pattern in the file given by path
	Writes to STDOUT, in the following format
	[path to file] : [line containing the pattern]
	
	@params:
		path : Path to the file to be searched
		pattern : Pattern to be searched
*/
void searchPatternInFile(char* path, char* pattern);

/**
 * Add iNode to list if it is unique to the list.
 * 
 * @param arr   array of iNodes
 * @param size  size of arr
 * @param end   the end of added iNodes, after this data is junk
 * @param iNode the iNode element to add
 * @return      1 if element was added, 0 if it wasn't
 */
int addINodeToListIfUnique(ino_t** arr, int size, int* end, ino_t iNode);

#endif