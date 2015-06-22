all: svctl

svctl: svctl.o
	gcc -std=c99 -Wall -pedantic -o svctl svctl.o

svctl.o: svctl.c secvault/svctl_ioctl.h
	gcc -std=c99 -Wall -pedantic -c svctl.c


clean:
	rm -rf svctl svctl.o svctl.c~
