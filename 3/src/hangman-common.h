/**
 * @file hangman-common.h
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 14.05.2015
 *
 * @brief This module defines common constants and macros for both hangman-server and hangman-client.
 **/
 
 #ifndef HANGMAN_COMMON_H
 #define HANGMAN_COMMON_H
 
/* === Constants === */
#define MAX_ERROR (9)

/* === Macros === */
#ifdef _ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif
 
 
#endif // HANGMAN_COMMON_H
