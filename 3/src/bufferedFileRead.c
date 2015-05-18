/**
 * @file bufferedFileRead.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 20.03.2015
 *
 * @brief Implementation of the bufferedFileRead module
 **/

#include "bufferedFileRead.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int readFile(FILE *f, struct Buffer *buffer, size_t maxLineLength, bool all_characters) {
	
	char tmpBuffer[maxLineLength];
	char *linePointer;
	size_t lineLength = 0;
	int c;
	
		
	while ( feof(f) == 0 ) {
		
		c = fgetc(f);
		if (lineLength + 1 >= maxLineLength || ferror(f) != 0 ) {
			return -1;
		}
		
		if ( (c == '\n' || c == '\r' || c == EOF) && lineLength > 0 ) {
			/* correctly terminate the String */
			tmpBuffer[lineLength] = '\0';
			lineLength++;
			
			/* resize buffer->content to fit one more string pointer */
			if( (buffer->content = realloc(buffer->content, 
			    (buffer->length + 1) * sizeof(char **))) == NULL) {
				return -1;
			}
			/* initialize memory for a new String to store the characters of the current line */
			if( (linePointer = calloc( lineLength, sizeof (char)) ) == NULL ) {
				return -1;
			};
		
			strncpy(linePointer, &tmpBuffer[0], lineLength);
			buffer->content[buffer->length] = linePointer;
			buffer->length++;
			lineLength = 0;
					
		} else {
			if (all_characters || isalpha(c) || c == ' ') {
				tmpBuffer[lineLength] = (char)toupper(c);
				lineLength++;
			}
		}		
	}
	
	return 0;
}


void freeBuffer(struct Buffer *buffer) {
	
	for (int i=0; i < buffer->length; i++) {
		free(buffer->content[i]);
	}
	
	free(buffer->content);	
	return;
}
