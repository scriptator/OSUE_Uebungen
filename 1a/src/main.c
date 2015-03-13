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
#include "main.h"

const char* pgm_name = "mysort";

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static int usage()
{
	fprintf(stderr,"USAGE: %s [-r] [file1] ...\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Program entry point
 * @brief
 * @details
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_FAILURE if any error occurs and 0 otherwise.
 **/
int main(int argc, const char** argv) 
{
	usage();
	
	exit(EXIT_SUCCESS);
}
