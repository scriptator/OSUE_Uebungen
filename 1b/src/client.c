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
#include <signal.h>
#include <limits.h>

/* === Constants === */
#define GUESS_BYTES (2)
#define RESPONSE_BYTES (1)
#define PARITY_ERR_BIT (6)
#define GAME_LOST_ERR_BIT (7)

#define EXIT_PARITY_ERROR (2)
#define EXIT_GAME_LOST (3)
#define EXIT_MULTIPLE_ERRORS (4)

#define SLOTS (5)
#define COLORS (8)
#define STATUS_WIDTH (3)
#define SHIFT_WIDTH (3)


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

/* the possible colors in the game */
enum colors {beige = 0, darkblue, green, orange, red, black, violet, white};


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
static uint8_t *read_from_server(int fd, uint8_t *buffer, size_t n);

/**
 *
 */
static void free_resources(void);

/**
 *
 */
static uint16_t calculate_guess(uint8_t *colors);


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
	
	/* Parse portno to see if it is a number and in a valid range (not required) */
	char *endptr;
    long portno = strtol(params->port, &endptr, 10);

    if ((errno == ERANGE && (portno == LONG_MAX || portno == LONG_MIN)) || (errno != 0 && portno == 0)) {
        bail_out(EXIT_FAILURE, "strtol: %s", strerror(errno));
    }

    if (endptr == params->port) {
        bail_out(EXIT_FAILURE, "No digits were found");
    }

    /* If we got here, strtol() successfully parsed a number */

    if (*endptr != '\0') { /* In principle not necessarily an error... */
        bail_out(EXIT_FAILURE,
            "Further characters after <secret-port>: %s", endptr);
    }

    /* check for valid port range */
    if (portno < 1 || portno > 65535) {
        bail_out(EXIT_FAILURE, "Use a valid TCP/IP port range (1-65535)");
    }
	
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

static uint8_t *read_from_server(int fd, uint8_t *buffer, size_t n)
{
    /* loop, as packet can arrive in several partial reads */
    size_t bytes_recv = 0;
    do {
        ssize_t r;
        r = recv(fd, buffer + bytes_recv, n - bytes_recv, 0);
        if (r <= 0) {
            return NULL;
        }
        bytes_recv += r;
    } while (bytes_recv < n);

    if (bytes_recv < n) {
        return NULL;
    }
    return buffer;
}

static uint16_t calculate_guess(uint8_t *colors) {
	
	uint16_t guess = 0;
	uint8_t parity = 0;
	uint8_t cur_color = 0;
	
	// encode the colors and calculate the parity
	for (int i = SLOTS - 1; i >= 0; i--) {
		
		cur_color = colors[i];
		if (cur_color < 0 || cur_color > COLORS) {
			bail_out(EXIT_FAILURE, "Tried to send invalid color");
		}
		
		guess <<= SHIFT_WIDTH;
		guess |= cur_color;
		parity ^= cur_color ^ (cur_color >> 1) ^ (cur_color >> 2);
	}
	
	parity &= 0x1;
	guess |= (parity << (8 * GUESS_BYTES - 1));
	
	return guess;
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
	
	
	/* set up the socket */
	int sockfd;
	sockfd = open_client_socket(params.hostname, params.port);
	
	
	/* Game Variable Declarations */
	uint8_t guessColors[SLOTS] = {red, green, orange, black, violet};
	uint16_t guess;
	uint8_t response;
	int ret = EXIT_SUCCESS;
	int error = 0;
	int round = 0;
	uint8_t red;
	uint8_t white;
	
	/* play the game */
	while (!error) {
		round++;
		
		// compute next guess
		// guessColors = TODO: plug in game strategy algorithm
		guess = calculate_guess(guessColors);
		
		// send guess to server
		DEBUG("Round %d: Sending guess 0x%x\n", round, guess);
		if ( send(sockfd, (const void *)&guess, GUESS_BYTES, 0) < 0) {
			bail_out(EXIT_FAILURE, strerror(errno));
		}
		
		// recieve server answer
		if (read_from_server(sockfd, &response, RESPONSE_BYTES) == NULL) {
            bail_out(EXIT_FAILURE, "read_from_server failed");
		}
		DEBUG("Server response: 0x%x\n", response);
		
		red = (response << (SHIFT_WIDTH + STATUS_WIDTH)) >> (SHIFT_WIDTH + STATUS_WIDTH);
		white = (response << SHIFT_WIDTH) >> (SHIFT_WIDTH + STATUS_WIDTH);
		if (red == SLOTS) {
			ret = EXIT_SUCCESS;
			printf("Rounds: %d\n", round);
			break;
		}
		
		// check for parity error or game over
		if (response & (1<<PARITY_ERR_BIT)) {
			(void) fprintf(stderr, "%s: Parity error\n", progname);	
			error = 1;
			ret = EXIT_PARITY_ERROR;
		}
		if (response & (1 << GAME_LOST_ERR_BIT)) {
			(void) fprintf(stderr, "%s: Game lost\n", progname);
         	error = 1;
         	if (ret == EXIT_PARITY_ERROR) {
            	ret = EXIT_MULTIPLE_ERRORS;
         	} else {
            	ret = EXIT_GAME_LOST;
         	}
     	}
	}
	
	/* done */
	free_resources();
	return ret;
}
