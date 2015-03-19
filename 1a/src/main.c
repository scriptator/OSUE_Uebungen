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
	ascending = -1,	/**< Sorting direction ascending */
	descending = 1 /**< Sorting direction descending */
};
static enum Direction sortingDirection = ascending; /** global sorting direction variable */

struct Buffer
{
	char **adress;
	int length;
};

/**
 *
 */
static void errorExit(char *errorMessage) {
	fprintf(stderr, "%s\n", errorMessage);
	exit(EXIT_FAILURE);
}

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
 *
 */
static void printStringArray(char **arr, size_t size) {
	
	for(int i=0; i < size; i++) {
		printf("%p: %s", arr[i], arr[i]);
	}
}



/**
 * @brief this function compares two strings
 * @details this function compares two strings by calling library method strncmp
 * Setting the global sortingDirection variable to descending makes this function return the inverse comparison result.
 * @param *str1 pointer to the first string
 * @param *str2 pointer to the second string
 * @return <0 ==0 >0
 **/
static int compareStrings(const void *a, const void *b) {
	const char *ia = (const char *)a;
	const char *ib = (const char *)b;
	printf("adress1: %p adress2: %p\n", ia, ib);
	printf("comparing %s with %s\n", ia, ib);
	printf("length1: %lu, length2: %lu\n", strlen(ia), strlen(ib));
	return sortingDirection * strncmp(ia,ib,INPUT_LINE_LENGTH);
}

/**
 *
 */
static void readFiles(char **paths, size_t size, struct Buffer *buffer) {
	
	char tmpBuffer[1024];
	char *linePointer;
	FILE *f;
		
	for(int i=0; i < size; i++) {

		if( (f = fopen(paths[i], "r")) == NULL ) {
	   		errorExit("fopen failed");
		}
		
		while (fgets(tmpBuffer, INPUT_LINE_LENGTH, f) != NULL) {
			if( (buffer->adress = realloc(buffer->adress, (buffer->length + 1) * sizeof(char **))) == NULL) {
				errorExit("buffer reallocation failed");
			}
			if( (linePointer = calloc( /*strlen(tmpBuffer) + 1*/ INPUT_LINE_LENGTH, sizeof (char)) ) == NULL ) {
				errorExit("linePointer allocation failed");
			};		
			
			strncpy(linePointer, tmpBuffer, /*strlen(tmpBuffer) + 1*/ INPUT_LINE_LENGTH);
			buffer->adress[buffer->length] = linePointer;
			buffer->length++;
		}
		
		if (ferror(f)) {
		     errorExit("fgets failed");
		}
		if (fclose(f) != 0) { 
			errorExit("fclose failed");
		}
	}
	
	return;
}

/**
 * Program entry point
 * @brief
 * @details
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_FAILURE if any error occurs and 0 otherwise.
ar**/
int main(int argc, char **argv) 
{
	char c; // character for option parsing
 	struct Buffer *buffer = malloc(sizeof (struct Buffer));
	buffer->adress = malloc(sizeof (char **));
	
	// TODO: check definedness of buffer
	
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
				
		int fileCount = argc - optind;
		char *filePaths[fileCount];
		
		/* parse the rest of the argument list for files to sort */
		for(int i=0; i<fileCount; i++) {
			filePaths[i] = argv[i + optind];
			//printf("%s", filePaths[i]);
		}
		
		/* read them into the buffer */
		readFiles(filePaths, fileCount, buffer);
		
		
	} else {	/* there are no files --> read from stdin */
	    /*
		char buffer[1024];
		
	    while (fgets(buffer, sizeof(buffer), stdin) != 0)
	    {
	        char *line = malloc(1024);
	        if (line == 0) {
	            break;
			}
	        strncpy(line, buffer, 1024);
	        cur_len++;
			
			char **newptr = (char **) realloc(input, sizeof(char**) * cur_len);
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
		
		*/
	    //printf("%s [%d]", input, (int)strlen(input));
	}
	
	
	// call the sorting method
	qsort(buffer->adress, buffer->length, sizeof(char *) * INPUT_LINE_LENGTH, compareStrings);
	
	// print to stdout		
	printStringArray(buffer->adress, buffer->length);
	
	return(EXIT_SUCCESS);
}
