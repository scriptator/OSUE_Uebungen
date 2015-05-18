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

/* === Constants === */
#define GALLOW_PATH ("./files/galgen")
#define GALLOW_EXTENSION ("txt")
#define GALLOW_LINE_LENGTH (64)

/* === Global Variables === */

/* Name of the program */
static const char *progname = "hangman-client"; /* default name */

/* Signal indicator */
volatile sig_atomic_t caught_sig = 0;

/* Buffer array storing the gallows ascii images */
static struct Buffer gallows[MAX_ERROR + 1];

/* Shared memory */
static struct Hangman_SHM *shared;

/* The number of this client, is determined by the server */
static int clientno = -1;

/* Semaphores for client-server synchonisation */
static sem_t *srv_sem = NULL;
static sem_t *clt_sem = NULL;
static sem_t *ret_sem = NULL;

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

/**
 * @brief Prints the first "size" strings of a given char** array to stdout, where size must not 
 * be greater than the size of the array. 
 * @param **arr The String array.
 * @param size The number of lines to print. Must not be greater than the size of the array.
 * @return nothing
 */
static void printStringArray(char **arr, size_t size);


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
    
    for (int i=0; i <= MAX_ERROR; i++) {
    	freeBuffer(&gallows[i]);
    }
	
	if (shared != NULL) {
		if (munmap(shared, sizeof *shared) == MAP_FAILED) {
			(void) fprintf(stderr, "munmap: %s\n", strerror(errno));
		}
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
}


static void printStringArray(char **arr, size_t size) 
{
	for(int i=0; i < size; i++) {
		(void) printf("%s\n", arr[i]);
	}
}

static void output_score(unsigned int i, char *tried_chars, char *server_response) {
	printStringArray(gallows[i].content, gallows[i].length);
	(void) printf ("\nYou have now tried: %s", tried_chars);
	(void) printf (" and the word is %s\n\n", server_response);
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
	
	/****** Reading the gallow images into the buffer array *******/
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
		if (fclose(f) != 0) { 
			free(path);
			bail_out(EXIT_FAILURE, "fclose failed on file %s", path);
		}	
	}
	DEBUG("done\n");
	
	/******* Initialization of SHM *******/
	DEBUG("SHM initialization\n");
	
	int shmfd = shm_open(SHM_NAME, O_RDWR , PERMISSION);
	if (shmfd == -1) {
		bail_out(EXIT_FAILURE, "No server accessible");
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
	
	/********************* Start game **************************/
	DEBUG("Starting Game\n");
	unsigned int round = 0;
	unsigned int errors = 0;
	char c;
	char buf[MAX_WORD_LENGTH];
	char tried_chars[MAX_WORD_LENGTH];
	enum GameStatus game_status = New;	

	while(caught_sig == 0) {
		if (game_status == Open) {
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
			
		DEBUG("Sending game Status %d\n", game_status);	
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
			(void) printf("You played with all the available words \n");
			break;
		}
		output_score(errors, tried_chars, buf);
		
		if (game_status != Open) {
			if (game_status == Won)  (void) printf("Congratulations! You figured it out.\n");
			if (game_status == Lost) (void) printf("Game Over! Want to try again?\n");
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
	// TODO: win-loss Stand
	
	free_resources();
	return EXIT_SUCCESS;
}
