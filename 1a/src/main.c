#include <stdio.h>
#include <stdlib.h>
#include "main.h"

int main(int argc, const char** argv) 
{
	printf("this program is to be implemented\n");
	usage();
	
	exit(EXIT_SUCCESS);
}

int usage()
{
	printf("Usage: mysort [-r] [file1] ...\n");
	exit(EXIT_FAILURE);
}
