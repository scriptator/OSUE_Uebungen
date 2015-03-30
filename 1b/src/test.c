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

/* Port number */
static const char *PORT = "1234";

/* Name of the program with default value */
static const char *progname = "test";

/* Client and Server pipes */
static FILE *clt;
static FILE *srv;

static void free_resources() {
	pclose(clt);
	pclose(srv);
}

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

    free_resources();
    exit(exitcode);
}


static void runTest()
{
	char clt_buff[256];
	char srv_buff[256];
	
	char *clt_invocation = "/Users/johannesvass/Studium/2015S_OSUE/OSUE_Uebungen/1b/build/client localhost 1234 2>&1";
	char *srv_invocation = "/Users/johannesvass/Studium/2015S_OSUE/OSUE_Uebungen/1b/build/server 1234 rgosv 2>&1";

	
	// create secret
	const char *secret = "rgosv";
	DEBUG("created secret \"%s\"\n", secret);
	
	//(void) sprintf(srv_invocation, "./server %s %s", PORT, secret);
	//(void) sprintf(clt_invocation, "./client localhost %s", PORT);
	DEBUG("%s\n", clt_invocation);
	
	// open client and server processes
	if( ((srv = popen(srv_invocation, "r")) == NULL) || ((clt = popen(clt_invocation, "r")) == NULL) ) {
		bail_out(EXIT_FAILURE, "popen failed");
	}
	
	while((fgets(clt_buff, sizeof(srv_buff), clt) != NULL) && (fgets(srv_buff, sizeof(srv_buff), srv) != NULL)) {
		DEBUG("client: %s\n", clt_buff);
		DEBUG("server: %s\n", srv_buff);
	}
	
	pclose(clt);
	pclose(srv);
	
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
	if ((errno == ERANGE && (num_it == INT_MAX || num_it == INT_MIN))
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
		runTest();
	}

	return EXIT_SUCCESS;
}
