/**
 * @file main.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 13.03.2015
 *
 * @brief Write sorted concatenation of all FILE(s) to standard output.
 * @details This program sorts multiple input files and prints the concatenation to standart output.
 * If no file is specified data is read from stdin.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define INPUT_LINE_LENGTH (1024)
#define PROGRAM_NAME ("mysort")	/* the programm name */

/**
 * @brief This is the enumeration used for describing sorting directions
 **/
enum Direction
{
	ascending = 1,	/**< Sorting direction ascending */
	descending = -1 /**< Sorting direction descending */
};
static enum Direction sortingDirection = ascending; /** global sorting direction variable */

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr and exits with EXIT_FAILURE
 * @details global variables: pgm_name
 * @return void
 **/
static void usage()
{
	(void) fprintf(stderr,"USAGE: %s [-r] [file1] ...\n", PROGRAM_NAME);
	exit(EXIT_FAILURE);
}


/**
 * @brief this function compares two chars
 * @details this function compares two chars by comparing the values of their ASCII represenations. 
 * Setting the global sortingDirection variable to descending makes this function return the difference b-a.
 * @param *a pointer to the first character
 * @param *b pointer to the second character
 * @return the difference (a-b) between the two chars in ASCII representation
 **/
static int compareChars(const void *a, const void *b) {
	return (sortingDirection * (*(int*)a - *(int*)b));
}


/**
 * Program entry point
 * @brief
 * @details
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_FAILURE if any error occurs and 0 otherwise.
 **/
int main(int argc, char **argv) 
{
	/* parse options using getopt */
	char c;
	
	while ( (c = getopt(argc, argv, "r")) != -1 ) {
		switch(c) {
			case 'r': /* absteigend sortieren */
				sortingDirection = descending;
				break;
			case '?': /* ungueltiges Argument */
				usage();
				break;
			default:  /* unmöglich */
				assert(0);
		} 
	}
	
	/* parse the rest of the argument list for files to sort */
	for(int i=optind; i<argc; i++) {
		char *filePath = argv[i];
		//TODO: open and read the file
	}
	
	//TODO: call the sorting method
	//qsort(array,numberofelmts,INPUT_LINE_LENGTH, compareChars);
		
	return(EXIT_SUCCESS);
}
