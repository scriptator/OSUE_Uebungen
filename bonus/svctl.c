#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include "secvault/svctl_ioctl.h"

#define SECVAULT_CONTROL_FILE ("/dev/sv_ctl")

/*************** Globals ***************/

static int fd = -1;

/*************** Prototypes ***************/

static void bail_out(const char *fmt, ...);
static void usage(void);
static void read_key(char *key);

/************* Implementations **************/

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

	if (fd >= 0) 
		(void) close(fd);
	
    exit(EXIT_FAILURE);
}


static void usage() {
	(void) fprintf(stderr, "USAGE: ./svctl [-c <size>|-k|-e|-d] <secvault id>\n");
	exit(EXIT_FAILURE);
}

static void read_key(char *key)
{
	char buf[SECVAULT_KEY_LENGTH + 1];
	(void) memset(buf, 0, sizeof buf);
	
	if (fgets(buf, SECVAULT_KEY_LENGTH, stdin) == NULL) 
		bail_out("Could not read key");

	memcpy(key, &buf[0], SECVAULT_KEY_LENGTH);
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
				if (endptr == optarg || size < 1 || size > 1048576) {
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
	
	if (optind == argc) {
		(void) fprintf(stderr, "Please specify a secvault id (between 0 and 3)\n");
		usage();
	}
	
	char *endptr;
	int sv_id = strtol (argv[optind], &endptr, 10);
	if (endptr == argv[optind] || sv_id < 0 || sv_id > 3) {
		(void) fprintf(stderr, "Please enter a valid secvault id (between 0 and 3)\n");
		usage();
	}
	
	/* open control file */
	fd = open(SECVAULT_CONTROL_FILE, 0);
	if (fd < 0) {
		bail_out("could not open secvault control file");
	}
	
	/* trigger ioctl calls */
	struct ioctl_data call_data;
	(void) memset (&call_data, 0, sizeof call_data);
	call_data.dev_nr = sv_id;
	
	if (argc == 2) {		// get size of secvault
		if (ioctl(fd, SV_GET_SIZE, &call_data) < 0) {
			bail_out("Could not get size of device %d", sv_id);
		}
		(void) printf("Size of device %d is %ld\n", sv_id, call_data.size);
		
	} else {
		
		if (opt_c == 1) {				/****** create secvault  *********/
			call_data.size = size;
			read_key(call_data.key);
			
			if (ioctl(fd, SV_CREATE_SECVAULT, &call_data) < 0) {
				bail_out("Could not create device %d.", sv_id);
			}
			(void) printf("Successfully created device %d with size %ld.\n", sv_id, call_data.size);
		}
		
		if (opt_k == 1) {				/********* change key ************/
			read_key(call_data.key);
			
			if (ioctl(fd, SV_CHANGE_KEY, &call_data) < 0) {
				bail_out("Could not change key of device %d.", sv_id);
			}
			(void) printf("Successfully changed key of device %d.\n", sv_id);
		}
		
		if (opt_e == 1) {				/********* wipe secvault **********/
			if (ioctl(fd, SV_WIPE_SECVAULT, &call_data) < 0) {
				bail_out("Could not wipe device %d.", sv_id);
			}
			(void) printf("Successfully wiped device %d.\n", sv_id);
		}
		
		if (opt_d == 1) {				/************ delete secvault ***********/
			if (ioctl(fd, SV_DELETE_SECVAULT, &call_data) < 0) {
				bail_out("Could not delete device %d.", sv_id);
			}
			(void) printf("Successfully deleted device %d.\n", sv_id);
		}
	}
	
	(void) close(fd);
	return EXIT_SUCCESS;
}
