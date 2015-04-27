/**
 * @file mygzip.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 27.04.2015
 *
 * @brief The command-line utility mygzip is capable of reading data 
 * to compress from stdin and write it to stdout or the given file
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

/* === Macros === */
#ifdef _ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif


/* === Constants === */
#define CHILD1_INVOCATION ( "gzip" )
#define CHILD1_ARGS ( "-cf" )
#define BUFFER_SIZE ( 128 )


/* === Global Variables === */

/** @brief Name of the program with default value */
static char *progname = "mygzip";

/** @brief Ouput File */
static FILE *output_file = NULL;

/** @brief Pipe for writing the data from gzip to the ouput file */
static int output_pipe[2];

/** @brief The read end of the ouput_pipe */
static FILE *output_pipe_read = NULL;

/** @brief Pipe for writing the data from stdin to the gzip process */
int input_pipe[2];

/** @brief The write end of the input_pipe */
static FILE *input_pipe_write = NULL;


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
 * Flushes the target-stream before return but does not fclose(3) it.
 * @param *source source file already opened for reading
 * @param *target target file already opened for writing
 */   
static void write_through(FILE *source, FILE *target);

/**
 * @brief free all the used resources, i.e. close files and pipes
 * @details global variables: output_file, output_pipe, output_pipe_read, input_pipe, input_pipe_write
 */
static void free_resources(void);

/**
 * @brief main execution function for child1. Calls dup2 for stdin and stdout then executes gzip 
 * @details global_variables: input_pipe, output_pipe
 */
static void child1_main(void);

/**
 * @brief main execution function for child2. Writes data from the gzip child to ouput_file
 * @details global_variables: output_file, output_pipe
 */
static void child2_main(void);


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
	
	DEBUG("Shutting down now.\n");
	free_resources();
    exit(exitcode);
}

static void free_resources(void)
{
	DEBUG("Freeing resources\n");
	if (output_file != NULL) {
		(void) fclose(output_file);
	}	
	if (output_pipe_read != NULL) {
		(void) fclose(output_pipe_read);	
	}
	if (input_pipe_write != NULL) {
		(void) fclose(input_pipe_write);
	}
		
	(void) close(input_pipe[0]);
	(void) close(input_pipe[1]);
	(void) close(output_pipe[0]);
	(void) close(output_pipe[1]);
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
	
	if ( fflush(target) == EOF) {
		bail_out(EXIT_FAILURE, "fflush error");
	}
}

static void child1_main(void)
{
	(void) close(input_pipe[1]);
	(void) close(output_pipe[0]);
	
	if ( dup2(input_pipe[0], fileno(stdin)) == -1) {
		bail_out(EXIT_FAILURE, "dup2");
	}
	if ( dup2(output_pipe[1], fileno(stdout)) == -1) {
		bail_out(EXIT_FAILURE,"dup2");
	}
	(void) execlp(CHILD1_INVOCATION, "mygzip_child1", CHILD1_ARGS, (char *) 0);
	bail_out (EXIT_FAILURE, "can‘t exec" );
}

static void child2_main(void)
{
	progname = "mygzip_child2";
	close(output_pipe[1]);
	
	if ( (output_pipe_read = fdopen(output_pipe[0], "r")) == NULL ) {
		bail_out(EXIT_FAILURE, "could not fdopen pipe");
	}
	write_through(output_pipe_read, output_file);

	/* clean up and exit */
	if ( fclose(output_file) == EOF) {
		bail_out(EXIT_FAILURE, "ouput_file could not be closed");
	}
	(void) fclose(output_pipe_read);
	(void) close(output_pipe[0]);
	exit(EXIT_SUCCESS);
}

/**
 * @brief Program entry point. Mygzip is a utility for reading data from stdin, compress it 
 * with gzip(1) and write the result to the given file or stdout, if no file is given.
 * @details global variables: all the variables (i.e. progname, output_file, output_pipe, output_pipe_read, input_pipe and input_pipe_write)
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS if no error occurs, otherwise execution is stopped via exit(2)
 */
int main (int argc, char **argv)
{
	/********* Argument parsing ***********/	
	if (argc > 0) {
		progname = argv[0];
		output_file = stdout;
	}
	if (argc == 2) {
		if ( (output_file = fopen(argv[1], "w")) == NULL) {
			bail_out(EXIT_FAILURE, "Could not open file %s for writing", argv[1]);
		}
	}
	if (argc > 2) {
		(void) fprintf(stderr, "SYNOPSIS: %s [file]\n", progname);
		exit(EXIT_FAILURE);
	}
	
	/* register exit_handler to free resources before program termination*/
	
	/*********** Child Creation ************/
	/* create output-pipe */
	if ( pipe(&output_pipe[0]) != 0) {
		bail_out(EXIT_FAILURE, "can't create output_pipe");
	}
	
	/* Create child2 (reads data from gzip and writes it to output_file) */
	pid_t pid_child2;
	switch  ( pid_child2 = fork() ) {	/* fork child 2 */
		case -1: 
			bail_out(EXIT_FAILURE, "can‘t fork" ); 
			break;
		case 0:		/* child: ChildProcess() */
			child2_main();	// does not return
			assert(0);
			break;
		default:	/* parent: go on with creating child1 */
			break;
	}
	
	/* create input-pipe (dont create together with the output-pipe because it is irrelevant to child2) */
	if ( pipe(&input_pipe[0]) != 0) {
		bail_out(EXIT_FAILURE, "can't create input_pipe");
	}
	
	/* Create child1 and exec gzip(2) */
	pid_t pid_child1;
	switch  ( pid_child1 = fork() ) {	/* fork child 1 */
		case -1: 
			bail_out(EXIT_FAILURE, "can‘t fork" ); 
			break;
		case 0:		/* child1: executes gzip */
			child1_main(); // does not return
			assert(0);
			break;
		default:	/* parent */
			break;	/* go on with this program */	
	}
	assert( (getpid() != pid_child1) && (getpid() != pid_child2) );
	
	/*********** parent process only: read from stdin and write to input_pipe to compress ************/
	(void) close(input_pipe[0]);
	(void) close(output_pipe[0]);
	(void) close(output_pipe[1]);
	
	/* open the write end of input_pipe as file and write data through */
	if ( (input_pipe_write = fdopen(input_pipe[1], "w")) == NULL ) {
		bail_out(EXIT_FAILURE, "could not fdopen input-pipe");
	}
	write_through(stdin, input_pipe_write);
	
	if ( fclose(input_pipe_write) == EOF) {
		bail_out(EXIT_FAILURE, "could not fclose the write end of input_pipe");
	}
	input_pipe_write = NULL;
	(void) close(input_pipe[1]);
	
	/* wait for children and check their exit codes */
	int status;
	pid_t pid;
	
	for (int childno=1; childno <=2; childno++) {
		pid = wait(&status);
		if(pid == pid_child1) {
			if ( WEXITSTATUS(status) != EXIT_SUCCESS ) {
				bail_out(EXIT_FAILURE, "gzip returned with an error");
			}
		} else if (pid == pid_child2) {
			if ( WEXITSTATUS(status) != EXIT_SUCCESS ) {
				bail_out(EXIT_FAILURE, "copy child returned with an error");
			}
		} else {
			bail_out(EXIT_FAILURE, "wait");
		}
	}
	
	/* print newline if writing to stdout for better layout */
	if (output_file == stdout) {
		(void) printf("\n");
	}
	
	return EXIT_SUCCESS;
}
