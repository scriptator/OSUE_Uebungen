CC=gcc
OSUEFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g
BUILDDIR=build/

mysort: mysort.c
	$(CC) $(OSUEFLAGS) -o $(BUILDDIR)$@ $^

clean:
	rm -rf build/*
	