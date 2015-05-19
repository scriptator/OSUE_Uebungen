/**
 * @file hangman-common.h
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 14.05.2015
 *
 * @brief This module defines common constants and macros for both hangman-server and hangman-client.
 **/ 
#ifndef HANGMAN_COMMON_H
#define HANGMAN_COMMON_H
 
#include <stdbool.h>
#include <stdint.h>

/* === Constants === */
#define MAX_ERROR (9)						/**< The number of errors a client can make until the game is over */
#define MAX_WORD_LENGTH (50)				/**< The maximum length a word to guess may be (according to Wikipedia the longest word is 45 chars long) */

#define PERMISSION (0600)							/**< UNIX file permission for semaphores and shm */
#define SHM_NAME ( "/1327476_hangman_shm" )			/**< name of the shared memory */
#define SRV_SEM ( "/1327476_hangman_srv_sem" )		/**< name of the server semaphore */
#define CLT_SEM ( "/1327476_hangman_clt_sem" )		/**< name of the client semaphore */
#define RET_SEM ( "/1327476_hangman_ret_sem" )		/**< name of the return semaphore */

/* === Structures === */

/**
 * @brief Enumeration describing the various states a game can be in.
 */
enum GameStatus {
	New,			/**< Set by client -> Requests a new game */
	Open,			/**< Set by server after deciding upon a new word */
	Impossible,		/**< Set by server if a new game is requested and no unplayed word is available */
	Lost,			/**< Set by server if MAX_ERROR is exceeded */ 
	Won,			/**< Set by server if the word is guessed correctly */
};

/**
 * @brief Structure used for client-server communication. Will lie in shared memory and allow client and server
 * to exchange all the relevant informations needed for gameplay.
 */
struct Hangman_SHM {
	unsigned int errors;			/**< the number of errors that this client already made */
	int clientno;					/**< the number identifying the clien */
	enum GameStatus status;			/**< the status the game is in */
	char tried_char;				/**< the character guessed by the client */
	char word[MAX_WORD_LENGTH];		/**< the answer of the server – a partly unobscured word */
	bool terminate;					/**< a flag to allow client and server tell each other if they terminated */
};


/* === Macros === */
#ifdef _ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

/* Length of an array */
#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))
 
#endif // HANGMAN_COMMON_H
