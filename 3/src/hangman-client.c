/**
 * @file hangman-server.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 10.05.2015
 * 
 * @brief This module is a client to the hangman-server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "bufferedFileRead.h"
#include "hangman-common.h"
#include "gallows.h"

/* === Constants === */
#define GALLOW_PATH ("./files/galgen")
#define GALLOW_EXTENSION ("txt")
#define GALLOW_LINE_LENGTH (64)


/* === Global Variables === */

/** @brief Name of the program */
static const char *progname = "hangman-client"; /* default name */

/** @brief Signal indicator, gets set to 1 on SIGINT or SIGTERM */
volatile sig_atomic_t caught_sig = 0;

/** @brief The ID of this client, gets decided by the server on registration */
static int clientno = -1;

/** @brief Shared memory for client-server communication */
static struct Hangman_SHM *shared;
/** @brief Semaphores which tells the server when there is a request */
static sem_t *srv_sem;
/** @brief Semaphores which tells the clients when the server is ready */
static sem_t *clt_sem;
/** @brief Semaphores which tells the client who has sent the last request when there is an answer */
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
 * @param soft if true, the server is informed about the shutdown, else the resources are free'd silently
 */
static void free_resources(bool soft);

/**
 * @brief Signal handler
 * @param sig Signal number catched
 */
static void signal_handler(int sig) 
{
	caught_sig = 1;
}


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

    free_resources(true);
    exit(exitcode);
}

static void free_resources(bool soft)
{
	DEBUG("Freeing resources\n");
    
	if (soft && shared != NULL) {
		/*** Begin critical Section: sending termination info ***/
		if (sem_wait(clt_sem) == -1) {
			if (errno != EINTR) (void) fprintf(stderr, "%s: sem_wait\n", progname);
			else (void) fprintf(stderr, "%s: interrupted while trying to inform server about shutdown\n", progname);
		} else {
			DEBUG("Sending termination info\n");	
			shared->terminate = true;
			shared->clientno = clientno;
	
			if (sem_post(srv_sem) == -1) (void) fprintf(stderr, "%s: sem_post\n", progname);
		}
		/*** End critical Section: sending termination info ***/
	}
	
	if (shared != NULL) {
		if (munmap(shared, sizeof *shared) == -1) {
			(void) fprintf(stderr, "%s: munmap: %s\n", progname, strerror(errno));
		}
	}
	
	if (sem_close(srv_sem) == -1) (void) fprintf(stderr, "%s: sem_close on %s: %s\n", progname, SRV_SEM, strerror(errno));
	if (sem_close(clt_sem) == -1) (void) fprintf(stderr, "%s: sem_close on %s: %s\n", progname, CLT_SEM, strerror(errno));
	if (sem_close(ret_sem) == -1) (void) fprintf(stderr, "%s: sem_close on %s: %s\n", progname, RET_SEM, strerror(errno));
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
	if (argc != 1) {
		fprintf(stderr, "No command line arguments allowed.\nUSAGE: %s", progname);
		exit(EXIT_FAILURE);
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
	
	/******* Initialization of SHM *******/
	DEBUG("SHM initialization\n");
	
	int shmfd = shm_open(SHM_NAME, O_RDWR , PERMISSION);
	if (shmfd == -1) {
		fprintf(stderr, "%s: No server accessible. Start hangman-server first!\n", progname);
		exit(EXIT_FAILURE);
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
	srv_sem = sem_open(SRV_SEM, 0); 
	clt_sem = sem_open(CLT_SEM, 0);
	ret_sem = sem_open(RET_SEM, 0);
	if (srv_sem == SEM_FAILED || clt_sem == SEM_FAILED || ret_sem == SEM_FAILED) {
		bail_out(EXIT_FAILURE, "sem_open");
	}
	
	/****** Reading the gallow images into the buffer array *******/
	/*
	DEBUG("Reading gallows ... ");
	FILE *f;
	char *path;
	
	for (int i = 0; i <= MAX_ERROR; i++) {
		(void) asprintf(&path, "%s%d.%s", GALLOW_PATH, i, GALLOW_EXTENSION);
		
		if( (f = fopen(path, "r")) == NULL ) {
			free(path);
	   		bail_out(EXIT_FAILURE, "fopen failed on file %s", path);
		}
		if ( readFile(f, &gallows[i], GALLOW_LINE_LENGTH, true) != 0) {
			free(path);
			(void) fclose(f);
			bail_out(EXIT_FAILURE, "Error while reading file %s", path);
		};
		free(path);
		if (fclose(f) != 0) { 
			bail_out(EXIT_FAILURE, "fclose failed on file %s", path);
		}	
		
	}
	DEBUG("done\n"); */
	
	/********************* Start game **************************/
	DEBUG("Starting Game\n");
	unsigned int round = 0;
	unsigned int errors = 0;
	unsigned int wins = 0;
	unsigned int losses = 0;
	char c = '\0';
	char buf[MAX_WORD_LENGTH];
	char tried_chars[MAX_WORD_LENGTH];
	enum GameStatus game_status = New;	

	while(caught_sig == 0) {
		if (game_status == Open) {
			(void) printf("Please enter a letter you want to try ");
			if (fgets(&buf[0], MAX_WORD_LENGTH, stdin) == NULL) {
				if (errno == EINTR) continue;
				bail_out(EXIT_FAILURE, "fgets");
			}
			c = toupper(buf[0]);
		
			if (buf[1] != '\n') {
				(void) printf("Please enter only one letter.\n");
				continue;
			}
			if (!isalpha(c)) {
				(void) printf("Please enter a valid letter.\n");
				continue;
			}
		
			bool validChar = true;
			for (int i=0; i < round; i++) {
				if (tried_chars[i] == c) {
					validChar = false;
					break;
				}
			}
			if (!validChar) {
				(void) printf("Please enter letter you have not tried yet.\n");
				continue;
			}
			
			tried_chars[round] = c;
			round++;
		}
		
		/*** Begin critical Section: sending request ***/
		if (sem_wait(clt_sem) == -1) {
			if (errno == EINTR) continue;
			bail_out(EXIT_FAILURE, "sem_wait");
		}
			
		if(shared->terminate) {
			DEBUG("Server terminated. Shutting dowm.\n");
			free_resources(false);
			exit(EXIT_FAILURE);
		}

		shared->status = game_status;
		shared->clientno = clientno;
		shared->tried_char = c;
		
		if (sem_post(srv_sem) == -1) {
			bail_out(EXIT_FAILURE, "sem_post");
		}
		/*** End critical Section: sending request ***/
		
		/*** Begin critical Section: recieving answer ***/
		if (sem_wait(ret_sem) == -1) {
			if (errno == EINTR) {
				continue;
				(void) sem_post(clt_sem);
			}
			bail_out(EXIT_FAILURE, "sem_wait");
		}
		
		clientno = shared->clientno;
		strncpy(buf, shared->word, MAX_WORD_LENGTH);
		errors = shared->errors;
		game_status = shared->status;
		
		if (sem_post(clt_sem) == -1) {
			bail_out(EXIT_FAILURE, "sem_post");
		}
		/*** End critical Section: recieving answer ***/
		
		if (game_status == Impossible) {
			(void) printf("You played with all the available words. \n");
			break;
		} 
		
		(void) printf("%s", gallows[errors]);
		
		if (game_status == Open) {
			(void) printf ("\n%s ... you have already tried the following characters \"%s\"\n", buf, tried_chars);
		
		} else {
			(void) printf("The word was %s\n", buf);
		
			if (game_status == Won)  {
				(void) printf("Congratulations! You figured it out.\n");
				wins++;
			}
			if (game_status == Lost) {
				(void) printf("Game Over! Want to try again?\n");
				losses++;
			}
			(void) printf("You have now won %d games and lost %d.\n", wins, losses);
			(void) printf("Press 'y' to start a new game or 'n' to stop playing.\n");
			
			c = tolower(fgetc(stdin));
			if (ferror(stdin)) bail_out(EXIT_FAILURE, "fgetc");
			
			if (c == 'y') {
				game_status = New;
				round = 0;
				errors = 0;
				(void) memset(tried_chars, 0, sizeof tried_chars);
			} else {
				break;
			}
		}
	}
	
	/* no next game possible or client wants quit */
	(void) printf("You have won %d games and lost %d. Bye bye!\n", wins, losses);
	
	free_resources(true);
	return EXIT_SUCCESS;
}
