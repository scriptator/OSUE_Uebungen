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
#include <time.h>

#ifdef _ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

#define SLOTS (5)
#define COLORS (8)

/* the possible colors in the game */
enum colors {beige = 0, darkblue, green, orange, red, black, violet, white};

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

static void print_colors(uint8_t *colors, char *dest) 
{
	for (int i=0; i < SLOTS; i++) {
		switch (colors[i])
		{
			case beige: 
				dest[i] = 'b';
				break;
			case darkblue: 
				dest[i] = 'd';
				break;
			case green: 
				dest[i] = 'g';
				break;
			case orange: 
				dest[i] = 'o';
				break;
			case red: 
				dest[i] = 'r';
				break;
			case black: 
				dest[i] = 's';
				break;
			case violet: 
				dest[i] = 'v';
				break;
			case white: 
				dest[i] = 'w';
				break;
		}
	}
	dest[SLOTS] = '\0';
}


static int runTest(int testno)
{
	char clt_buff[256];
	char srv_buff[256];
	
	char cwd[FILENAME_MAX];
	char clt_invocation[FILENAME_MAX + 30];
	char srv_invocation[FILENAME_MAX + 30];

	
	// create secret
	uint8_t colors[SLOTS];
	srand(5*time(NULL) + 111*testno);
	for (int i=0; i < SLOTS; i++) {
		colors[i] = (rand() >> 5*i) % COLORS;
	}
	
	char secret[SLOTS + 1];
	print_colors(colors, &secret[0]);
	DEBUG("created secret \"%s\" out of %d\n", secret, rand() % COLORS);
	
	// Format the client and server invocation strings
	(void) getcwd(cwd, FILENAME_MAX);
	(void) sprintf(srv_invocation, "%s/build/server %s %s", cwd, PORT, secret);
	(void) sprintf(clt_invocation, "%s/build/client localhost %s", cwd, PORT);
	
	// open client and server processes
	if( ((srv = popen(srv_invocation, "r")) == NULL) || ((clt = popen(clt_invocation, "r")) == NULL) ) {
		bail_out(EXIT_FAILURE, "popen failed");
	}
	
	char *endptr;
	int rounds = 0;
	if((fgets(clt_buff, sizeof(srv_buff), clt) != NULL) && (fgets(srv_buff, sizeof(srv_buff), srv) != NULL)) {
		DEBUG("client: %s", clt_buff);
		DEBUG("server: %s", srv_buff);
		rounds = strtol(&clt_buff[7], &endptr, 10);
	}
		
	pclose(clt);
	pclose(srv);
	
	return rounds;
}

/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return {einfügen}
 */
int main(int argc, char *argv[])
{
	/* Parse Arguments */
	if (argc > 0) {
		progname = argv[0];
	}
	if (argc != 2) {
		(void) fprintf(stderr, "Usage: %s <number of iterations>\n", progname);
		return EXIT_FAILURE;
	}
	
	/* Parse number of iterations */
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
	int rounds = 0;
	for (int i=0; i<num_it; i++) {
		int cur_rounds = runTest(i);
		(void) printf("Running test %d: took %d rounds\n", i+1, cur_rounds);
		rounds += cur_rounds;
	}
	
	(void) printf("Average Rounds: %f\n", (double)rounds / (double)num_it);

	return EXIT_SUCCESS;
}
