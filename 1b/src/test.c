/**
 * @file test.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 21.03.2015
 *
 * @brief This application is used to test the speed of the mastermind algorithm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>

#ifdef _ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

/* Name of the program with default value */
static const char *progname = "test";

/**
 * @brief terminate program on program error
 * @param exitcode exit code
 * @param fmt format string
 */
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

    //free_resources();
    exit(exitcode);
}


/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return {einfügen}
 */
int main(int argc, char *argv[])
{
	if (argc > 0) {
		progname = argv[0];
	}
	if (argc != 2) {
		(void) fprintf(stderr, "Usage: %s <number of iterations>\n", progname);
		return EXIT_FAILURE;
	}
	
	// parse number of iterations
	char *endptr;
	int num_it = strtol(argv[1], &endptr, 10);
	if ((errno == ERANGE && (num_it == LONG_MAX || num_it == LONG_MIN))
         || (errno != 0 && num_it == 0)) {
       		bail_out(EXIT_FAILURE, "strtol");
    	}
	if (endptr == argv[1]) {
		bail_out(EXIT_FAILURE, "no digits were found");
	}
	DEBUG("number of iterations: %d\n", num_it);

	// execute client and server programs
	for (int i=0; i<num_it; i++) {
		(void) printf("Running test %d\n", i+1);
	}

	return EXIT_SUCCESS;
}
