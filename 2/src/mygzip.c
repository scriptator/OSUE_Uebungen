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
#include <sys/wait.h>

/* === Constants === */
#define CHILD1_INVOCATION ( "gzip" )
#define CHILD1_ARGS ( "-cf" )
#define BUFFER_SIZE ( 128 )

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
 * @details global variables: ???
 * @param *source source file already opened for reading
 * @param *target target file already opened for writing
 */   
static void write_through(FILE *source, FILE *target);

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

static void write_through(FILE *source, FILE *target)
{
	uint8_t buffer[BUFFER_SIZE];
	size_t bytes_read;
	
	while ( (bytes_read = fread(&buffer, sizeof (buffer[0]), BUFFER_SIZE, source)) == BUFFER_SIZE ) {
		if ( fwrite(&buffer, sizeof (buffer[0]), BUFFER_SIZE, target) < BUFFER_SIZE) {
			bail_out(EXIT_FAILURE, "fwrite error");
		}
	}
	if ( ferror(source) != 0) {		/* an error occurred */
		bail_out(EXIT_FAILURE, "fread error");
	}
	else if ( fwrite(&buffer, sizeof (buffer[0]), bytes_read, target) < bytes_read) {	/* EOF recieved, write the last few bytes */
		bail_out(EXIT_FAILURE, "fwrite error");
	}
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
	
	switch  ( pid_child1 = fork() ) {	/* fork child 1 */
		case -1: 
			bail_out(EXIT_FAILURE, "can‘t fork" ); 
			break;
		case 0:		/* child1: executes gzip */
			close(input_pipe[1]);
			close(output_pipe[0]);
			dup2(input_pipe[0], fileno(stdin));
			dup2(output_pipe[1], fileno(stdout));
			execlp(CHILD1_INVOCATION, "gzip", CHILD1_ARGS, (char *) 0);
			bail_out (EXIT_FAILURE, "can‘t exec" );
			break;
		default:	/* parent */
			break;	/* go on with this program */	
	}
	
	switch  ( pid_child2 = fork() ) {	/* fork child 2 */
		
		case -1: 
			bail_out(EXIT_FAILURE, "can‘t fork" ); 
			break;
			
		case 0:		/* child: ChildProcess() */
			close(input_pipe[0]);
			close(input_pipe[1]);
			close(output_pipe[1]);
			
			FILE *output_pipe_read;
			if ( (output_pipe_read = fdopen(output_pipe[0], "r")) == NULL ) {
				bail_out(EXIT_FAILURE, "could not fdopen pipe");
			}
			write_through(output_pipe_read, output_file); 
			
			exit(EXIT_SUCCESS);
			break;
			
		default:	/* parent: go on after switch statement*/
			break;
	}
		
	/* parent process: read from stdin and write to input_pipe to compress */
	assert( (getpid() != pid_child1) && (getpid() != pid_child2) );
	close(input_pipe[0]);
	close(output_pipe[0]);
	close(output_pipe[1]);
	
	FILE *input_pipe_write;
	if ( (input_pipe_write = fdopen(input_pipe[1], "w")) == NULL ) {
		bail_out(EXIT_FAILURE, "could not fdopen pipe");
	}
	write_through(stdin, input_pipe_write);
	fclose(input_pipe_write);
	
	/* wait for children */
	int status;
	pid_t pid;
	
	for (int childno=1; childno <=2; childno++) {
		pid = wait(&status);
		if(pid == pid_child1) {
			if (WEXITSTATUS(status) != EXIT_SUCCESS ) {
				bail_out(EXIT_FAILURE, "gzip returned with an error");
			}
		} else if (pid == pid_child2) {
			if (WEXITSTATUS(status) != EXIT_SUCCESS ) {
				bail_out(EXIT_FAILURE, "copy child returned with an error");
			}
		} else {
			bail_out(EXIT_FAILURE, "wait");
		}
	}
	
	exit(EXIT_SUCCESS);
}
