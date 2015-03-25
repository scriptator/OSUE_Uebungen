/**
 * @file bufferedFileRead.h
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 20.03.2015
 *
 * @brief Module for reading the content of a file into a struct buffer defined in this header.
 * @details This module contains methods for reading from a file into a buffer and freeing the buffer afterwards.
 **/

#ifndef BUFFEREDFILEREAD_H
#define BUFFEREDFILEREAD_H

#include <stdio.h>

/**
 * @brief A structure to store lines of strings.
 */
struct Buffer
{
	char **content;	/**< Pointer to the String array. */
	int length;	/**< Can be used to keep track of the current array size. */
};

/**
 * @brief Reads the content of a FILE* into a struct buffer.
 * @details The content of FILE* is read line by line into the specified Buffer*. The size of 
 * buffer->content gets increased for every line and buffer->length gets incremented.
 * @param *f The already opened file to read from.
 * @param *buffer A struct of type Buffer to store the data in.
 * @param maxLineLength The maximum length of a line that can be read from the file
 * @return A value different from 0 if an error occurs during memory allocation, 0 otherwise.
 */
int readFile(FILE *f, struct Buffer *buffer, size_t maxLineLength);


/**
 * @brief Frees the allocated space of a buffer and all the content inside
 * @param *buffer A pointer to the buffer to be freed.
 * @return nothing
 */
void freeBuffer(struct Buffer *buffer);

#endif /* BUFFEREDFILEREAD_H */
