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
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define INPUT_LINE_LENGTH (1024)
#define PROGRAM_NAME ("mysort")	/* the programm name */

/**
 * @brief This is the enumeration used for describing sorting directions
 */
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
	char c; // character for option parsing
    char **input = malloc(sizeof(char**));
	size_t cur_len = 0;
	
	/* parse options using getopt */	
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
	
	if(optind < argc) { /* there are files specified via command line arguments */
		/* parse the rest of the argument list for files to sort */
		for(int i=optind; i<argc; i++) {
			char *filePath = argv[i];
			//TODO: open and read the file
		}
	} else {	/* there are no files --> read from stdin */
	    char buffer[1024];
		
	    while (fgets(buffer, sizeof(buffer), stdin) != 0)
	    {
	        size_t buf_len = strlen(buffer);
	        char *line = malloc(buf_len + 1);
	        if (line == 0) {
	            break;
			}
	        strcpy(line, buffer);
	        cur_len++;
			
			char **newptr = realloc(input, sizeof(char**) * cur_len);
	        if (newptr == 0) {
	            break;
			}
			input = newptr;
			input[cur_len - 1] = line;
			
			printf("line pointer is: %p\n", line);
			printf("line is: %s", input[cur_len - 1]);
			printf("input length: %lu\n", (sizeof input));
			printf("current length: %zu\n\n", cur_len);
	    }
		
	    //printf("%s [%d]", input, (int)strlen(input));
	}
	
	
	//TODO: call the sorting method
	qsort(input,cur_len,1024,compareChars);		
	
	/* output to stdout */
	for(int i = 0; i < cur_len; i++) {
		printf("%s",input[i]);
	}
	
	return(EXIT_SUCCESS);
}
