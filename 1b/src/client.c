/**
 * @file client.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 21.03.2015
 *
 * @brief 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


/* === Constants === */

/* === Macros === */

#ifdef _ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

/* === Global Variables === */

/* Name of the program with default value */
static const char *progname = "client";

/* Socket file descriptor */
static int sockfd = -1;


/* === Type Definitions === */

/* structure to store the program arguments */
struct client_params {
	char *hostname;
	char *port;
};

/* === Prototypes === */

/**
 *
 */
static void bail_out(int exitcode, const char *fmt, ...);

/**
 *
 */
static void parse_args(int argc, char *argv[], struct client_params *params);

/**
 *
 */
static int open_client_socket(const char *hostname, const char *port);

/**
 *
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

void parse_args(int argc, char *argv[], struct client_params *params) 
{
    if(argc > 0) {
        progname = argv[0];
    }
	if (argc != 3) {
		bail_out(EXIT_FAILURE, "Usage: %s <server-hostname> <secret-port>", progname);
	}
	
	progname = argv[0];
	params->hostname = argv[1];
	params->port = argv[2];
	
	return;
}

int open_client_socket(const char *hostname, const char *port)
{
	DEBUG("Opening socket\n");
	int sockfd = -1;
	int sock_error;
	const char *errcause = NULL;
	struct addrinfo hints;
	struct addrinfo *ai, *ai_head;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	sock_error = getaddrinfo(hostname, port, &hints, &ai_head);
	if (sock_error) {
		bail_out(EXIT_FAILURE, gai_strerror(sock_error));
	}
	
	for (ai = ai_head; ai; ai = ai->ai_next) {		
		sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sockfd < 0) {
			errcause = "Could not create socket";
			continue;
		}
		if ( connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0) {
			errcause = strerror(errno);
			(void) close(sockfd);
			sockfd = -1;
			continue;
		}
		
		break; /* we got a connection */
	}
	if(sockfd < 0) {
		bail_out(EXIT_FAILURE, errcause);
	}
	
	DEBUG("Successfully opened socket %d\n", sockfd);
	freeaddrinfo(ai_head);	
	return sockfd;
}

static void free_resources(void)
{
    /* clean up resources */
    DEBUG("Shutting down server\n");
    if(sockfd >= 0) {
        (void) close(sockfd);
    }
}

/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return {einfügen}
 */
int main(int argc, char *argv[])
{
	struct client_params params;
	parse_args(argc, argv, &params);
	
	int sockfd;
	sockfd = open_client_socket(params.hostname, params.port);
	
	int sentBytes = send(sockfd, "Test", strlen("Test"), 0);
	if(sentBytes < 0) {
		bail_out(EXIT_FAILURE, strerror(errno));
	}
	
	/* done */
	free_resources();
	return EXIT_SUCCESS;
}
