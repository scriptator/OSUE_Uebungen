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
#include <time.h>
#include "bufferedFileRead.h"
#include "hangman-common.h"

/* === Constants === */


/* === Structures and Enumerations === */

struct Game {
	char *secret_word;
	char obscured_word[MAX_WORD_LENGTH];
	enum GameStatus status;
	unsigned int errors;
};

struct Client {
	int clientno;
	struct WordNode *used_words;
	struct Game current_game;
	struct Client *next;
};

struct WordNode {
	char *word;
	struct WordNode *next;
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

/* client count for client number assignment */
static unsigned int client_count = 0;

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

static void new_game(struct Client *client);

static void free_words(struct Client *client);

static void calculate_results(struct Client *client, char try);

static bool contains(struct WordNode *node, char *word);


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

	if (sem_close(srv_sem) == -1) (void) fprintf(stderr, "sem_close on %s: %s\n", SRV_SEM, strerror(errno));
	if (sem_close(clt_sem) == -1) (void) fprintf(stderr, "sem_close on %s: %s\n", CLT_SEM, strerror(errno));
	if (sem_close(ret_sem) == -1) (void) fprintf(stderr, "sem_close on %s: %s\n", RET_SEM, strerror(errno));
	if (sem_unlink(SRV_SEM) == -1) (void) fprintf(stderr, "sem_unlink on %s: %s\n", SRV_SEM, strerror(errno));
	if (sem_unlink(CLT_SEM) == -1) (void) fprintf(stderr, "sem_unlink on %s: %s\n", CLT_SEM, strerror(errno));
	if (sem_unlink(RET_SEM) == -1) (void) fprintf(stderr, "sem_unlink on %s: %s\n", RET_SEM, strerror(errno));
}

static void signal_handler(int sig) 
{
	caught_sig = 1;
}

static void new_game(struct Client *client)
{
	srand(time(NULL));
	unsigned int pos = rand() % word_buffer.length;
	unsigned int i = 0;
	
	while (contains(client->used_words, word_buffer.content[pos])) {
		pos = (pos + 1) % word_buffer.length;
		if (i >= word_buffer.length) {
			client->current_game.status = Impossible;
			return;
		}
		i++;
	}
	
	(void) memset(&client->current_game, 0, sizeof (struct Game));
	client->current_game.secret_word = word_buffer.content[pos];
	client->current_game.status = Open;
	
	struct WordNode *used_word = malloc(sizeof (struct WordNode));
	if (used_word == NULL) {
		bail_out(EXIT_FAILURE, "malloc while creating a WordNode");
	}
	used_word->word = word_buffer.content[pos];
	used_word->next = client->used_words;
	client->used_words = used_word;
	
	for (i = 0; i < strnlen(client->current_game.secret_word, MAX_WORD_LENGTH - 1); i++) {
		if (client->current_game.secret_word[i] == ' ') {
			client->current_game.obscured_word[i] = ' ';
		} else {
			client->current_game.obscured_word[i] = '_';
		}
	}
	client->current_game.obscured_word[i+1] = '\0';
}

static void free_words(struct Client *client)
{
	struct WordNode *cur = client->used_words;
	struct WordNode *next;
	
	while (cur != NULL) {
		next = cur->next;
		free(cur);
		cur = next;
	}
}

static bool contains(struct WordNode *node, char *word)
{
	for (; node != NULL; node = node->next) {
		if (strncmp(node->word, word, MAX_WORD_LENGTH) == 0) {
			return true;
		}
	}
	return false;
}

static void calculate_results(struct Client *client, char try)
{
	bool error = true;
	bool won = true;
	
	/* unobscure secret string */
	for (size_t i = 0; i < strnlen(client->current_game.secret_word, MAX_WORD_LENGTH); i++) {
		if (client->current_game.secret_word[i] == try) {
			client->current_game.obscured_word[i] = try;
			error = false;
		}
		won = won && (client->current_game.obscured_word[i] != '_');
	}
	
	if (error) {
		client->current_game.errors++;
		if (client->current_game.errors >= MAX_ERROR) {
			client->current_game.status = Lost;
		}
	} else if (won) {
		client->current_game.status = Won;
	}
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
	struct Client *cur;
	
	while (caught_sig == 0) {
		
		/*** Begin critical Section ***/
		if (sem_wait(srv_sem) == -1) {
			if (errno == EINTR) continue;
			bail_out(EXIT_FAILURE, "sem_wait");
		}
		
		if (shared->clientno == -1) {	/* new client */
			cur = (struct Client *) calloc (1, sizeof (struct Client));
			if (cur == NULL) {
				bail_out(EXIT_FAILURE, "calloc on creation of new client");
			}
			cur->clientno = client_count++;
			cur->next = clients;
			clients = cur;
		} else {	/* existing client */
			cur = clients;
			while (cur != NULL && cur->clientno != shared->clientno) {
				cur = cur->next;
			}
			
			if (cur == NULL) bail_out(EXIT_FAILURE, "Could not find client with number %d", shared->clientno);
		}
		
		switch (shared->status) {
			case New:
				new_game(cur);
				break;
			case Open:
				calculate_results(cur, shared->tried_char);
				break;
			default:
				assert(0 && "status may only be in {New, Open}"); 
		}
		
		shared->clientno = cur->clientno;
		shared->status = cur->current_game.status;
		shared->errors = cur->current_game.errors;
		strncpy(shared->word, cur->current_game.obscured_word, MAX_WORD_LENGTH);
		
		DEBUG("clientno %d ... status: %d, errors: %d, secret: %s, obscured: %s\n", shared->clientno, shared->status, shared->errors, cur->current_game.secret_word, shared->word);
	
		if (sem_post(ret_sem) == -1) {
			bail_out(EXIT_FAILURE, "sem_post");
		}
		
		/*** End critical Section ***/
	}
	
	DEBUG("Caught signal, shutting down\n");
	free_resources();
	return EXIT_SUCCESS;
}
