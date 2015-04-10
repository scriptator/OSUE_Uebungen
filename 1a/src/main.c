/**
 * @file main.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 01.04.2015
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
#include <stdarg.h>
#include <assert.h>
#include <errno.h>

/* === Constants === */
#define INPUT_LINE_LENGTH (1024)	// 1022 without newline and \0

/* === Type Definitions === */
/**
 * @brief This is the enumeration used for describing sorting directions
 */
enum Direction
{
	ascending = 1,	/**< Sorting direction ascending */
	descending = -1 /**< Sorting direction descending */
};

/* === Global Variables === */

/**
 * @brief program name, "mysort" by default
 */
static char *programName = "mysort";

/**
 * @brief sorting direction variable, set to descending on -r option
 */
static enum Direction sortingDirection = ascending;

/**
 * @brief the input buffer
 */
static struct Buffer *buffer; 


/* === Function Prototypes === */

/**
 * @brief terminate program on program error
 * @details global variables: programName, buffer, errno
 * @param exitcode exit code
 * @param fmt format string
 */
static void bail_out(int exitcode, const char *fmt, ...);

/**
 * @brief Prints the first "size" strings of a given char** array to stdout, where size must not 
 * be greater than the size of the array. 
 * @param **arr The String array.
 * @param size The number of lines to print. Must not be greater than the size of the array.
 * @return nothing
 */
static void printStringArray(char **arr, size_t size);

/**
 * @brief this function compares two strings using library method strcmp
 * @details global variables: sortingDirection
 * @param *str1 pointer to the first string
 * @param *str2 pointer to the second string
 * @return If sorting direction is ascending, an integer greater than, equal to, or less than 0,
 * according as the string a is greater than, equal to, or less than the string b. 
 **/
static int compareStrings(const void *a, const void *b);


/* === Implementations === */

static void bail_out(int exitcode, const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "%s: ", programName);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    freeBuffer(buffer);
    exit(exitcode);
}

static void printStringArray(char **arr, size_t size) 
{
	for(int i=0; i < size; i++) {
		(void) printf("%s\n", arr[i]);
	}
}

static int compareStrings(const void *a, const void *b) 
{
	const char **ia = (const char **)a;
	const char **ib = (const char **)b;

	return sortingDirection * strcmp(*ia,*ib);
}


/**
 * @brief Entry point of mysort. Argument and option parsing. Calls the sorting method.
 * @details global variables: prograName, buffer, sortingDirection
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS, if no error occurs. Otherwise the programm is stopped via 
 * exit(EXIT_FAILURE).
ar**/
int main(int argc, char **argv) 
{
	programName = argv[0];
 	
	/* initialize the buffer */
	if( (buffer = malloc(sizeof (struct Buffer))) == NULL) {
		bail_out(EXIT_FAILURE, "Buffer initialization failed");
	};
	buffer->content = malloc(sizeof (char **));
	buffer->length = 0;
		
	/* parse options using getopt */	
	int c;
	while ( (c = getopt(argc, argv, "r")) != -1 ) {
		switch(c) {
			case 'r': /* absteigend sortieren */
				sortingDirection = descending;
				break;
			case '?': /* ungueltiges Argument */
				bail_out(EXIT_FAILURE, "USAGE: %s [-r] [file1] ...", programName);
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
		   		bail_out(EXIT_FAILURE, "fopen failed on file %s", path);
			}
			if ( readFile(f, buffer, INPUT_LINE_LENGTH) != 0) {
				bail_out(EXIT_FAILURE, "Error while reading file %s", path);
			};
			if (fclose(f) != 0) { 
				bail_out(EXIT_FAILURE, "fclose failed on file %s", path);
			}
		}		
		
	} else {	/* there are no files --> read from stdin */
		if ( readFile(stdin, buffer, INPUT_LINE_LENGTH) != 0) {
			bail_out(EXIT_FAILURE, "Memory allocation error while reading from stdin");
		};
	}

	qsort_r(buffer->content, buffer->length, sizeof(char *), compareStrings);
	printStringArray(buffer->content, buffer->length);
	freeBuffer(buffer);
	
	return(EXIT_SUCCESS);
}
