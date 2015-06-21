#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>

#define KEY_LENGTH (10)

static void bail_out(const char *fmt, ...);
void usage(void);

static void bail_out(const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "svctl: ");
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));
    }
    (void) fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}


void usage() {
	(void) fprintf(stderr, "USAGE: ./svctl [-c <size>|-k|-e|-d] <secvault id>\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
	if (argc == 1) {
		usage();
	}
	
	char opt;
	int opt_k = 0;
	int opt_e = 0;
	int opt_d = 0;
	int opt_c = 0;
	int size = 0;
	
	while ( (opt = getopt(argc, argv, "c:ked")) != -1) {
		switch (opt) {
		    case 'c':
				opt_c++;
				char *endptr;
				size = strtol (optarg, &endptr, 10);
				if (endptr != optarg || size >= 1 || size <= 1000000) {
					(void) fprintf(stderr, "Size not valid.\n");
					usage();
				}
		 	    break;
			case 'k':
				opt_k++;
				break;
			case 'e':
				opt_e++;
				break;
			case 'd':
				opt_d++;
				break;
		    default: /* '?' */
		 	    usage();
		}
	}
	
	if (opt_c > 1 || opt_k > 1 || opt_e > 1 || opt_d > 1) {
		(void) fprintf(stderr, "Specify option only one time.\n");
		usage();
	}
	
	

	return EXIT_SUCCESS;
}

