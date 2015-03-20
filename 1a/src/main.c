/**
 * @file main.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 20.03.2015
 *
 * @brief Write sorted concatenation of all FILE(s) to standard output.
 * @details This program sorts multiple input files and prints the concatenation to standart output.
 * If no file is specified data is read from stdin.
 **/

#include "bufferedFileRead.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define INPUT_LINE_LENGTH (1024)
static char *programName;

/**
 * @brief This is the enumeration used for describing sorting directions
 */
enum Direction
{
	ascending = 1,	/**< Sorting direction ascending */
	descending = -1 /**< Sorting direction descending */
};
static enum Direction sortingDirection = ascending; /** global sorting direction variable */


/**
 * @brief Stops program execution by calling exit(3)
 * @param *errorMessage The string to display before terminating.
 */
static void errorExit(char *errorMessage) {
	(void) fprintf(stderr, "%s exited with message: %s\n", programName, errorMessage);
	exit(EXIT_FAILURE);
}

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr and exits with EXIT_FAILURE
 * @details global variables: PROGRAM_NAME
 * @return nothing
 **/
static void usage()
{
	(void) fprintf(stderr,"USAGE: %s [-r] [file1] ...\n", programName);
	exit(EXIT_FAILURE);
}

/**
 * @brief Prints an array of strings to stdout.
 * @details Prints the first size strings of a given char** array to stdout, where size must not be greater than the size of the array. 
 * @param **arr The String array.
 * @param size The number of lines to print. Must not be greater than the size of the array.
 * @return nothing
 */
static void printStringArray(char **arr, size_t size) {
	
	for(int i=0; i < size; i++) {
		(void) printf("%s\n", arr[i]);
	}
}

/**
 * @brief this function compares two strings
 * @details this function compares two strings by calling library method strcmp
 * Setting the global sortingDirection variable to descending makes this function return the inverse comparison result.
 * @param *str1 pointer to the first string
 * @param *str2 pointer to the second string
 * @return If sorting direction is ascending, an integer greater than, equal to, or less than 0, according as the string a is greater than, equal to, or less than the string b. 
 **/
static int compareStrings(const void *a, const void *b) {
	const char **ia = (const char **)a;
	const char **ib = (const char **)b;

	return sortingDirection * strcmp(*ia,*ib);
}


/**
 * @brief Entry point of mysort. Argument and option parsing. Calls the sorting method.
 * @details
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS, if no error occurs. Otherwise the programm is stopped via exit(EXIT_FAILURE).
ar**/
int main(int argc, char **argv) 
{
	programName = argv[0];
 	
	/* initialize the buffer */
	struct Buffer *buffer; 
	if( (buffer = malloc(sizeof (struct Buffer))) == NULL) {
		errorExit("Buffer initialization failed");
	};
	buffer->content = malloc(sizeof (char **));
	buffer->length = 0;
		
	/* parse options using getopt */	
	char c;
	while ( (c = getopt(argc, argv, "r")) != -1 ) {
		switch(c) {
			case 'r': /* absteigend sortieren */
				sortingDirection = descending;
				break;
			case '?': /* ungueltiges Argument */
				usage();
				break;
			default:  /* unmöglich */
				assert(0);
		} 
	}
	
	if(optind < argc) { /* there are files specified via command line arguments */
		int fileCount = argc - optind;
		char *path;
		FILE *f;
		
		/* parse the rest of the argument list for files to sort */
		for(int i=0; i<fileCount; i++) {
			
			path = argv[i + optind];
			
			if( (f = fopen(path, "r")) == NULL ) {
		   		errorExit("fopen failed");
			}
			if ( readFile(f, buffer, INPUT_LINE_LENGTH) != 0) {
				errorExit("Memory allocation error while reading file");
			};
			if (ferror(f) != 0) {
			     errorExit("fgets failed");
			}
			if (fclose(f) != 0) { 
				errorExit("fclose failed");
			}
		}		
		
	} else {	/* there are no files --> read from stdin */
		if ( readFile(stdin, buffer, INPUT_LINE_LENGTH) != 0) {
			errorExit("Memory allocation error while reading file");
		};
	}

	qsort(buffer->content, buffer->length, sizeof(char *), compareStrings);
	printStringArray(buffer->content, buffer->length);
	freeBuffer(buffer);
	
	return(EXIT_SUCCESS);
}
