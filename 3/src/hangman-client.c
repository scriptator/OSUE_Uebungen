/**
 * @file hangman-server.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 10.05.2015
 * 
 * @brief This module is a client to the hangman-server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include "bufferedFileRead.h"
#include "hangman-common.h"

/* === Constants === */
#define GALLOW_PATH ("./files/galgen")
#define GALLOW_EXTENSION ("txt")
#define GALLOW_LINE_LENGTH (64)

/* === Global Variables === */

/* Name of the program */
static const char *progname = "hangman-client"; /* default name */

/* Signal indicator */
static volatile sig_atomic_t caught_sig = -1;

/* Buffer array storing the gallows ascii images */
static struct Buffer gallows[MAX_ERROR + 1];

/* === Prototypes === */

/**
 * @brief terminate program on program error
 * @param exitcode exit code
 * @param fmt format string
 */
static void bail_out(int exitcode, const char *fmt, ...);

/**
 * @brief Signal handler
 * @param sig Signal number catched
 */
static void signal_handler(int sig);

/**
 * @brief free allocated resources
 */
static void free_resources(void);

/**
 * @brief Prints the first "size" strings of a given char** array to stdout, where size must not 
 * be greater than the size of the array. 
 * @param **arr The String array.
 * @param size The number of lines to print. Must not be greater than the size of the array.
 * @return nothing
 */
static void printStringArray(char **arr, size_t size);


/* === Implementations === */

static void bail_out(int exitcode, const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "%s: ", progname);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    free_resources();
    exit(exitcode);
}

static void free_resources(void)
{
    /* clean up resources */
    DEBUG("Shutting down server\n");
    // TODO: Implement
}


static void printStringArray(char **arr, size_t size) 
{
	for(int i=0; i < size; i++) {
		(void) printf("%s\n", arr[i]);
	}
}

/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return TODO
 */
int main(int argc, char *argv[])
{
	/****** Argument parsing ******/
	if (argc > 0) {
		progname = argv[0];
 	}
	if (argc != 1) {
		fprintf(stderr, "No command line arguments allowed.\nUSAGE: %s", progname);
		exit(EXIT_FAILURE);
	}
	
	/****** Reading the gallow images into the buffer array *******/
	FILE *f;
	char *path;
	
	for (int i = 0; i <= MAX_ERROR; i++) {
		(void) asprintf(&path, "%s%d.%s", GALLOW_PATH, i, GALLOW_EXTENSION);
		
		if( (f = fopen(path, "r")) == NULL ) {
			free(path);
	   		bail_out(EXIT_FAILURE, "fopen failed on file %s", path);
		}
		if ( readFile(f, &gallows[i], GALLOW_LINE_LENGTH) != 0) {
			free(path);
			bail_out(EXIT_FAILURE, "Error while reading file %s", path);
		};
		if (fclose(f) != 0) { 
			free(path);
			bail_out(EXIT_FAILURE, "fclose failed on file %s", path);
		}	
		
		printStringArray(gallows[i].content, gallows[i].length);
		sleep(1);
	}
	
	return EXIT_SUCCESS;
}
