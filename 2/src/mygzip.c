/**
 * @file mygzip.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 28.04.2015
 *
 * @brief //TODO: fill in
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

/* === Constants === */
#define CHILD1_INVOCATION ( "gzip" )
#define CHILD1_ARGS ( "-cf" )

/* === Macros === */


/* === Type Definitions === */


/* === Global Variables === */
static char *progname = "mygzip";  /* default */

/* === Prototypes === */

/**
 * @brief terminate program in case of an error
 * @details global variables: progname, errno
 * @param exitcode exit code
 * @param fmt format string
 */   
static void bail_out(int exitcode, const char *fmt, ...);

/**
 * @brief this function reads data from a source file and writes it to a target file. 
 * Exits with EXIT_SUCCESS on EOF, if no error occurs.
 * @details global variables: ???
 * @param *source source file already opened for reading
 * @param *target target file already opened for writing
 */   
static void write_to_file_and_exit(FILE *source, FILE *target);

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

    exit(exitcode);
}

/**
 * 
 */
int main (int argc, char **argv)
{
	/* Argument parsing */
	FILE *output_file = stdout;
	
	if (argc > 0) {
		progname = argv[0];
	}
	if (argc == 2) {
		if ( (output_file = fopen(argv[1], "w")) == NULL) {
			bail_out(EXIT_FAILURE, "Could not open file %s for writing", argv[1]);
		}
	}
	if (argc > 2) {
		bail_out(EXIT_FAILURE, "SYNOPSIS: %s [file]", progname);
	}

	
	/* create pipes */
	int input_pipe[2];
	if ( pipe(&input_pipe[0]) != 0) {
		bail_out(EXIT_FAILURE, "can't create input_pipe");
	}
	
	int output_pipe[2];
	if ( pipe(&output_pipe[0]) != 0) {
		bail_out(EXIT_FAILURE, "can't create output_pipe");
	}
	
	/* Create child processes */
	pid_t pid_child1;
	pid_t pid_child2;
	switch  ( pid_child1 = fork() ) {	/* child 1 */
		case -1: 
			bail_out(EXIT_FAILURE, "can‘t fork" ); 
			break;
		case 0:		/* child1: executes gzip */
			close(input_pipe[0]);
			close(output_pipe[1]);
			dup2(input_pipe[1], fileno(stdin));
			dup2(output_pipe[0], fileno(stdout));
			execlp(CHILD1_INVOCATION, CHILD1_ARGS, (char *) 0);
			bail_out (EXIT_FAILURE, "can‘t exec" );
			break;
		default:	/* parent */
			break;	/* go on with this program */	
	}
	
	switch  ( pid_child2 = fork() ) {	/* child 2 */
		case -1: 
			bail_out(EXIT_FAILURE, "can‘t fork" ); 
			break;
		case 0:		/* child: ChildProcess() */
			fclose(input_pipe[0]);
			fclose(input_pipe[1]);
			fclose(output_pipe[0]);
			write_to_file_and_exit(output_pipe[1], output_file); 
			assert(0);	/* does not return */
			break;
		default:	/* parent */
			break;
	}
	
	/* parent process: read from stdin and write to input_pipe to compress */
	
	
	/* cleaning up */
	
	return EXIT_SUCCESS;
}
