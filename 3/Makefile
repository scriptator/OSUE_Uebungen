# OSUE 2015S - Task 3
# Author: Johannes Vass
# Date: 07.05.2015

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c
LFLAGS=-std=c99 -pedantic -Wall -lrt -pthread -g
DEBUG= -D_ENDEBUG
BUILDDIR=build
VPATH = src

all: hangman-server hangman-client

hangman-server:  $(BUILDDIR)/bufferedFileRead.o $(BUILDDIR)/hangman-server.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

hangman-client: $(BUILDDIR)/bufferedFileRead.o $(BUILDDIR)/hangman-client.o
	$(CC) $(LFLAGS) -o $(BUILDDIR)/$@ $^

$(BUILDDIR)/%.o: %.c %.h
	$(CC) $(CFLAGS) $(DEBUG) $< -o $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG) $< -o $@

clean:
	rm -rf build/*	
