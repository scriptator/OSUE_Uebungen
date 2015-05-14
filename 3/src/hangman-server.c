/**
 * @file hangman-server.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 10.05.2015
 * 
 * @brief This module behaves as a server in the hangman game.
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
#include <assert.h>
#include "bufferedFileRead.h"
#include "hangman-common.h"

/* === Constants === */
#define INPUT_LINE_LENGTH (32)


/* === Global Variables === */

/* Name of the program */
static const char *progname = "hangman-server"; /* default name */

/** @brief word buffer */
static struct Buffer word_buffer;

/* Signal indicator */
static volatile sig_atomic_t caught_sig = -1;

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
    DEBUG("Freeing resources\n");
    freeBuffer(&word_buffer);
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
	if (argc > 2) {
		fprintf(stderr, "Too many files\nUSAGE: %s [input_file]", progname);
		exit(EXIT_FAILURE);
	}
			
	int c;
	while ( (c = getopt(argc, argv, "")) != -1 ) {
		switch(c) {
			case '?': /* ungueltiges Argument */
				fprintf(stderr, "USAGE: %s [input_file]", progname);
				exit(EXIT_FAILURE);
			default:  /* unmöglich */
				assert(0);
		} 
	}
	
	
	/****** Reading the words for the game *******/
	
	if(argc == 2) { /* there is a file specified via command line argument */
		
		FILE *f;
		char *path = argv[1];
			
		if( (f = fopen(path, "r")) == NULL ) {
	   		bail_out(EXIT_FAILURE, "fopen failed on file %s", path);
		}
		if ( readFile(f, &word_buffer, INPUT_LINE_LENGTH) != 0) {
			bail_out(EXIT_FAILURE, "Error while reading file %s", path);
		};
		if (fclose(f) != 0) { 
			bail_out(EXIT_FAILURE, "fclose failed on file %s", path);
		}	
		
	} else {	/* there are no files --> read from stdin */
		if ( readFile(stdin, &word_buffer, INPUT_LINE_LENGTH) != 0) {
			bail_out(EXIT_FAILURE, "Memory allocation error while reading from stdin");
		};
	}
	
	
	/******* Initialization (of semaphores etc.) *******/
	
	
	/******* Ready for accepting clients ********/
	
	
	free_resources();
	return EXIT_SUCCESS;
}
