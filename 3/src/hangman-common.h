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
#define MAX_ERROR (9)
#define MAX_WORD_LENGTH (32)

#define SHM_NAME ( "/hangman_shm" )
#define PERMISSION (0600)

#define SRV_SEM ( "/hangman_srv_sem" )
#define CLT_SEM ( "/hangman_clt_sem" )
#define RET_SEM ( "/hangman_ret_sem" )

/* === Structures === */

enum GameStatus {
	New,
	Open,
	Impossible,
	Lost,
	Won
};

struct Hangman_SHM {
	enum GameStatus status;
	int clientno;
	char tried_char;
	char word[MAX_WORD_LENGTH];
	unsigned int errors;
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
