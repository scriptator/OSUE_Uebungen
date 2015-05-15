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
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <stdbool.h>
#include <fcntl.h>
#include <semaphore.h>
#include "bufferedFileRead.h"
#include "hangman-common.h"

/* === Constants === */


/* === Structures and Enumerations === */

struct Client {
	int cltno;
	struct Game *games;
	struct Client *next;
};

struct Game {
	char *secret_word;
	struct Try tries[MAX_ERROR];
	struct Game *next;
};

/* === Global Variables === */

/* Name of the program */
static const char *progname = "hangman-server"; /* default name */

/** @brief word buffer */
static struct Buffer word_buffer;

/* Signal indicator */
volatile sig_atomic_t caught_sig = 0;

/* Linked List of clients */
static struct Client *clients = NULL;

/* Shared memory */
static struct Hangman_SHM *shared;

/* Semaphores for client-server synchonisation */
static sem_t *srv_sem;
static sem_t *clt_sem;
static sem_t *ret_sem;

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
	
	if (shared != NULL) {
		if (munmap(shared, sizeof *shared) == -1) {
			(void) fprintf(stderr, "munmap: %s\n", strerror(errno));
		}
	}
	if(shm_unlink(SHM_NAME) == -1) {
		(void) fprintf(stderr, "shm_unlink: %s\n", strerror(errno));
	}

	if (sem_close(srv_sem) == -1) {
		(void) fprintf(stderr, "sem_close on %s: %s\n", SRV_SEM, strerror(errno));
	}
	if (sem_close(clt_sem) == -1) {
		(void) fprintf(stderr, "sem_close on %s: %s\n", CLT_SEM, strerror(errno));
	}
	if (sem_close(ret_sem) == -1) {
		(void) fprintf(stderr, "sem_close on %s: %s\n", RET_SEM, strerror(errno));
	}
	if (sem_unlink(SRV_SEM) == -1) {
		(void) fprintf(stderr, "sem_unlink on %s: %s\n", SRV_SEM, strerror(errno));
	}
	if (sem_unlink(CLT_SEM) == -1) {
		(void) fprintf(stderr, "sem_unlink on %s: %s\n", CLT_SEM, strerror(errno));
	}
	if (sem_unlink(RET_SEM) == -1) {
		(void) fprintf(stderr, "sem_unlink on %s: %s\n", RET_SEM, strerror(errno));
	}
}

static void signal_handler(int sig) {
	caught_sig = 1;
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
	
    /****** set up signal handlers *******/
    const int signals[] = {SIGINT, SIGTERM};
    struct sigaction s;

    s.sa_handler = signal_handler;
    s.sa_flags   = 0;
    if(sigfillset(&s.sa_mask) < 0) {
        bail_out(EXIT_FAILURE, "sigfillset");
    }
    for(int i = 0; i < COUNT_OF(signals); i++) {
        if (sigaction(signals[i], &s, NULL) < 0) {
            bail_out(EXIT_FAILURE, "sigaction");
        }
    }
	
	/****** Reading the words for the game *******/
	DEBUG("Reading game dictionary ... ");
	
	if(argc == 2) { /* there is a file specified via command line argument */
		
		FILE *f;
		char *path = argv[1];
			
		if( (f = fopen(path, "r")) == NULL ) {
	   		bail_out(EXIT_FAILURE, "fopen failed on file %s", path);
		}
		if ( readFile(f, &word_buffer, MAX_WORD_LENGTH, false) != 0) {
			(void) fclose(f);
			bail_out(EXIT_FAILURE, "Error while reading file %s", path);
		};
		if (fclose(f) != 0) { 
			bail_out(EXIT_FAILURE, "fclose failed on file %s", path);
		}	
		
	} else {	/* there are no files --> read from stdin */
		if ( readFile(stdin, &word_buffer, MAX_WORD_LENGTH, false) != 0) {
			bail_out(EXIT_FAILURE, "Memory allocation error while reading from stdin");
			if (caught_sig) {
				DEBUG("Caught signal, shutting down\n");
				free_resources();
				exit(EXIT_FAILURE);
			}
		};
	}
	
	DEBUG("done\n");
	
	
	/******* Initialization of SHM *******/
	DEBUG("SHM initialization\n");
	
	int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, PERMISSION);
	if (shmfd == -1) {
		bail_out(EXIT_FAILURE, "Could not open shared memory");
	}
	
	if ( ftruncate(shmfd, sizeof *shared) == -1) {
		(void) close(shmfd);
		bail_out(EXIT_FAILURE, "Could not ftruncate shared memory");
	}
	shared = mmap(NULL, sizeof *shared, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if (shared == MAP_FAILED) {
		(void) close(shmfd);
		bail_out(EXIT_FAILURE, "Could not mmap shared memory");
	}
	if (close(shmfd) == -1) {
		bail_out(EXIT_FAILURE, "Could not close shared memory file descriptor");
	}
	
	
	/******* Initialization of Semaphores *******/
	DEBUG("Semaphores initialization\n");
	srv_sem = sem_open(SRV_SEM, O_CREAT | O_EXCL, PERMISSION, 0); 
	clt_sem = sem_open(CLT_SEM, O_CREAT | O_EXCL, PERMISSION, 1);
	ret_sem = sem_open(RET_SEM, O_CREAT | O_EXCL, PERMISSION, 0);
	if (srv_sem == SEM_FAILED || clt_sem == SEM_FAILED) {
		bail_out(EXIT_FAILURE, "sem_open");
	}
	
	
	/******* Ready for accepting clients ********/
	DEBUG("Server Ready!\n");
	
	while (caught_sig == 0) {
		if (sem_wait(srv_sem) == -1) {
			if (errno == EINTR) continue;
			bail_out(EXIT_FAILURE, "sem_wait");
		}
		printf("Here is the server\n");
		
		if (sem_post(ret_sem) == -1) {
			bail_out(EXIT_FAILURE, "sem_post");
		}
	}
	
	DEBUG("Caught signal, shutting down\n");
	free_resources();
	return EXIT_SUCCESS;
}
