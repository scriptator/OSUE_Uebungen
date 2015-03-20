/**
 * @file main.c
 * @author Johannes Vass <e1327476@student.tuwien.ac.at>
 * @date 20.03.2015
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
 * @brief A structure to store lines of strings.
 */
struct Buffer
{
	char **content;	/**< Pointer to the String array. */
	int length;		/**< Length can be used to keep track of the current array size. */
};

/**
 * @brief Stops program execution by calling exit(3)
 * @param *errorMessage The string to display before terminating.
 */
static void errorExit(char *errorMessage) {
	fprintf(stderr, "%s\n", errorMessage);
	exit(EXIT_FAILURE);
}

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr and exits with EXIT_FAILURE
 * @details global variables: PROGRAM_NAME
 * @return nothing
 **/
static void usage()
{
	(void) fprintf(stderr,"USAGE: %s [-r] [file1] ...\n", PROGRAM_NAME);
	exit(EXIT_FAILURE);
}

/**
 * @brief Prints an array of strings to stdout.
 * @details Prints the first size strings of a given char** array to stdout, where size must not be greater than the size of the array. 
 * @param **arr The String array.
 * @param size The number of lines to print. Must not be greater than the size of the array.
 * @return nothing
 */
static void printStringArray(char **arr, size_t size) {
	
	for(int i=0; i < size; i++) {
		printf("%s\n", arr[i]);
	}
}

/**
 * @brief this function compares two strings
 * @details this function compares two strings by calling library method strcmp
 * Setting the global sortingDirection variable to descending makes this function return the inverse comparison result.
 * @param *str1 pointer to the first string
 * @param *str2 pointer to the second string
 * @return If sorting direction is ascending, an integer greater than, equal to, or less than 0, according as the string a is greater than, equal to, or less than the string b. 
 **/
static int compareStrings(const void *a, const void *b) {
	const char **ia = (const char **)a;
	const char **ib = (const char **)b;
	//printf("content1: %p content2: %p\n", ia, ib);
	//printf("comparing %s with %s\n", *ia, *ib);
	//printf("length1: %lu, length2: %lu\n", strlen(ia), strlen(ib));
	return sortingDirection * strcmp(*ia,*ib);
}

/**
 * @brief Reads the content of a FILE* into a struct buffer.
 * @details The content of FILE* is read line by line into the specified Buffer*. The size of buffer->content gets increased for every line and buffer->length gets incremented.
 * @param *f The already opened file to read from.
 * @param *buffer A struct of type Buffer to store the data in.
 * @return nothing
 */
static void readFile(FILE *f, struct Buffer *buffer) {
	
	char tmpBuffer[1024];
	char *linePointer;
	size_t lineLength;
		
	while (fgets(tmpBuffer, INPUT_LINE_LENGTH, f) != NULL) {
		lineLength = strlen(tmpBuffer);
		
		/* resize buffer->content to fit one more string */
		if( (buffer->content = realloc(buffer->content, (buffer->length + 1) * sizeof(char **))) == NULL) {
			errorExit("buffer reallocation failed");
		}
		/* initialize a new String to store the characters of the current line */
		if( (linePointer = calloc( lineLength + 1, sizeof (char)) ) == NULL ) {
			errorExit("linePointer allocation failed");
		};
		/* if there was any line feed, override it with '\0' */
		if( tmpBuffer[lineLength-1] == '\n') {
			tmpBuffer[lineLength-1] = '\0';
		}
		
		strncpy(linePointer, tmpBuffer, lineLength + 1);
		buffer->content[buffer->length] = linePointer;
		buffer->length++;
	}
	
	return;
}

/**
 * @brief Entry point of mysort. Argument and option parsing. Calls the sorting method.
 * @details
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS, if no error occurs. Otherwise the programm is stopped via exit(EXIT_FAILURE).
ar**/
int main(int argc, char **argv) 
{
	char c; // character for option parsing
 	struct Buffer *buffer = malloc(sizeof (struct Buffer));
	buffer->content = malloc(sizeof (char **));
	
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
		char *path;
		FILE *f;
		
		/* parse the rest of the argument list for files to sort */
		for(int i=0; i<fileCount; i++) {
			
			path = argv[i + optind];
			if( (f = fopen(path, "r")) == NULL ) {
		   		errorExit("fopen failed");
			}
			
			readFile(f, buffer);
			
			if (ferror(f)) {
			     errorExit("fgets failed");
			}
			if (fclose(f) != 0) { 
				errorExit("fclose failed");
			}
		}		
		
	} else {	/* there are no files --> read from stdin */
		readFile(stdin, buffer);
	}

	// call the sorting method
	qsort(buffer->content, buffer->length, sizeof(char *), compareStrings);
	
	// print to stdout		
	printStringArray(buffer->content, buffer->length);
	
	return(EXIT_SUCCESS);
}
