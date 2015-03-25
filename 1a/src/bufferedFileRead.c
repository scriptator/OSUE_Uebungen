/**
 * @file bufferedFileRead.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 20.03.2015
 *
 * @brief Module for reading the content of a file into a struct buffer defined in bufferedFileRead.h
 * @details This module contains methods for reading from a file into a buffer and freeing the buffer afterwards.
 **/

#include "bufferedFileRead.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int readFile(FILE *f, struct Buffer *buffer, size_t maxLineLength) {
	
	char tmpBuffer[maxLineLength];
	char *linePointer;
	size_t lineLength;
		
	while (fgets(tmpBuffer, maxLineLength, f) != NULL) {
		lineLength = strlen(tmpBuffer);
		
		/* resize buffer->content to fit one more string */
		if( (buffer->content = realloc(buffer->content, 
		    (buffer->length + 1) * sizeof(char **))) == NULL) {
			return -1;
		}
		/* initialize a new String to store the characters of the current line */
		if( (linePointer = calloc( lineLength + 1, sizeof (char)) ) == NULL ) {
			return -1;
		};
		/* if there was any line feed, override it with '\0' */
		if( tmpBuffer[lineLength-1] == '\n') {
			tmpBuffer[lineLength-1] = '\0';
		}
		
		strncpy(linePointer, tmpBuffer, lineLength + 1);
		buffer->content[buffer->length] = linePointer;
		buffer->length++;
	}
	
	return 0;
}


void freeBuffer(struct Buffer *buffer) {
	
	for (int i=0; i < buffer->length; i++) {
		free(buffer->content[i]);
	}
	
	free(buffer->content);
	free(buffer);
	
	return;
}
